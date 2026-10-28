# run_bots.py
import asyncio
import logging
from player import Player

WS_URL = "ws://localhost:1123/ws"  # <-- change me

async def main():
    players = [Player(WS_URL, name=f"bot{i}") for i in range(5)]
    # Run all players concurrently; Ctrl-C to stop.
    tasks = [asyncio.create_task(p.run()) for p in players]
    try:
        await asyncio.gather(*tasks)
    except asyncio.CancelledError:
        pass
    finally:
        for p in players:
            await p.close()

if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(name)s: %(message)s",
    )
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("stopped")