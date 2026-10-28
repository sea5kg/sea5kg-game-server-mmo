import asyncio
import websockets
import json
import os
import random
import logging
from typing import Optional, Dict, Any

log = logging.getLogger("infinity")


class Player:
    """A single game client (one WebSocket connection = one player)."""

    def __init__(
        self,
        url: str,
        name: str = "bot",
        secret_dir: str = ".secrets",
        move_interval: float = 1.0,
        move_mode: str = "delta",  # "delta" or "absolute"
    ):
        self.url = url
        self.name = name
        self.secret_dir = secret_dir
        self.move_interval = move_interval
        self.move_mode = move_mode

        # State
        self.player_id: Optional[str] = None   # "n" from auth result
        self.secret: Optional[str] = None
        self.pos: Dict[str, int] = {"x": 0, "y": 0}
        self.known_players: Dict[str, Dict[str, int]] = {}

        # Runtime
        self._ws: Optional[websockets.WebSocketClientProtocol] = None
        self._msg_id = 0
        self._stop = asyncio.Event()
        self._tasks: list[asyncio.Task] = []

        # Make sure secret dir exists
        os.makedirs(self.secret_dir, exist_ok=True)

    @property
    def secret_path(self) -> str:
        return os.path.join(self.secret_dir, f"{self.name}.json")

    def load_secret(self) -> bool:
        if not os.path.exists(self.secret_path):
            return False
        try:
            with open(self.secret_path, "r") as f:
                data = json.load(f)
            self.player_id = data.get("n")
            self.secret = data.get("secret")
            log.info(f"[{self.name}] loaded secret for player {self.player_id}")
            return True
        except Exception as e:
            log.warning(f"[{self.name}] failed to load secret: {e}")
            return False

    def save_secret(self) -> None:
        if not self.secret or not self.player_id:
            return
        with open(self.secret_path, "w") as f:
            json.dump({"n": self.player_id, "secret": self.secret}, f)
        log.info(f"[{self.name}] saved secret for player {self.player_id}")

    async def send(self, payload: Dict[str, Any]) -> None:
        assert self._ws is not None, "not connected"
        msg = json.dumps(payload)
        log.debug(f"[{self.name}] >>> {msg}")
        await self._ws.send(msg)

    async def rpc(self, method: str, params: Dict[str, Any]) -> int:
        """Send a JSON-RPC style request, return the id used."""
        mid = self._msg_id
        self._msg_id += 1
        await self.send({
            "jsonrpc": "2.0",
            "id": mid,
            "method": method,
            "params": params,
        })
        return mid

    async def connect(self) -> None:
        log.info(f"[{self.name}] connecting to {self.url} ...")
        self._ws = await websockets.connect(self.url)

        # Auth
        self.load_secret()  # may populate player_id / secret
        auth_params: Dict[str, Any] = {}
        # If your server supports relogin via secret, uncomment:
        # if self.secret and self.player_id:
        #     auth_params = {"n": self.player_id, "secret": self.secret}

        await self.send({"id": "auth", "method": "auth", "params": auth_params})

        # Reader loop in background
        self._tasks.append(asyncio.create_task(self._reader()))
        self._tasks.append(asyncio.create_task(self._mover()))

    async def run(self) -> None:
        """Connect and block until stopped / connection closed."""
        await self.connect()
        try:
            await self._stop.wait()
        finally:
            await self.close()

    async def close(self) -> None:
        self._stop.set()
        for t in self._tasks:
            t.cancel()
        for t in self._tasks:
            try:
                await t
            except (asyncio.CancelledError, Exception):
                pass
        self._tasks.clear()
        if self._ws is not None:
            try:
                await self._ws.close()
            except Exception:
                pass
            self._ws = None

    async def _reader(self) -> None:
        try:
            async for raw in self._ws:
                log.debug(f"[{self.name}] <<< {raw}")
                try:
                    data = json.loads(raw)
                except json.JSONDecodeError:
                    log.warning(f"[{self.name}] non-JSON frame")
                    continue
                try:
                    await self._handle(data)
                except Exception as e:
                    log.exception(f"[{self.name}] handler error: {e}")
        except websockets.ConnectionClosed:
            log.info(f"[{self.name}] connection closed")
        finally:
            self._stop.set()

    async def _handle(self, data: Dict[str, Any]) -> None:
        # 1. Auth result
        if data.get("id") == "auth" and "result" in data:
            res = data["result"]
            self.player_id = res.get("n", self.player_id)
            self.secret = res.get("secret", self.secret)
            self.pos = {"x": res.get("x", 0), "y": res.get("y", 0)}
            log.info(
                f"[{self.name}] authed as {self.player_id} "
                f"@ ({self.pos['x']},{self.pos['y']})"
            )
            self.save_secret()
            return

        # 2. Initial roster
        if data.get("method") == "init_players":
            players = data["params"].get("p", {})
            self.known_players = players
            log.info(
                f"[{self.name}] init_players: {len(players)} total "
                f"({', '.join(players)})"
            )
            return

        # 3. Player events
        if data.get("method") == "players_events":
            events = data["params"].get("e", [])
            for ev in events:
                n = ev.get("n")
                if n == self.player_id:
                    # Server-confirmed update of our own position
                    self.pos = {"x": ev.get("x", self.pos["x"]),
                                "y": ev.get("y", self.pos["y"])}
                else:
                    log.info(
                        f"[{self.name}] event: {n} -> "
                        f"({ev.get('x')},{ev.get('y')})"
                    )
            # Merge full state snapshot if provided
            for n, p in data["params"].get("p", {}).items():
                self.known_players[n] = p
            return

        log.debug(f"[{self.name}] unhandled: {data}")

    def _next_move(self) -> Dict[str, int]:
        if self.move_mode == "absolute":
            # E.g. random walk in a small square around origin.
            return {"x": random.randint(-10, 10),
                    "y": random.randint(-10, 10)}
        # Default: delta
        return {"x": random.choice([-10, 0, 10]),
                "y": random.choice([-10, 0, 10])}

    async def _mover(self) -> None:
        # Wait until we know who we are
        while self.player_id is None and not self._stop.is_set():
            await asyncio.sleep(0.1)
        if self._stop.is_set():
            return

        log.info(f"[{self.name}] movement loop started")
        try:
            while not self._stop.is_set():
                move = self._next_move()
                try:
                    await self.rpc("change_player_position", move)
                except websockets.ConnectionClosed:
                    break
                await asyncio.sleep(self.move_interval)
        except asyncio.CancelledError:
            raise
        finally:
            log.info(f"[{self.name}] movement loop stopped")