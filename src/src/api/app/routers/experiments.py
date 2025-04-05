import subprocess
import asyncio
import re

from fastapi import APIRouter, HTTPException
from fastapi.responses import JSONResponse
from pydantic import BaseModel

router = APIRouter(
    prefix="/experiments",
)

async def execute_tpg(cmd):
    process = await asyncio.create_subprocess_exec(
        *cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    output_lines = []
    async for line in process.stdout:
        output_lines.append(line.decode().strip())

    async for line in process.stderr:
        output_lines.append(line.decode().strip())

    return_code = await process.wait() 

    return output_lines, return_code

def extract_seed_and_pid(log_messages):
    """
    Extracts the seed and PID from the given array of log messages.

    Args:
        log_messages (list): A list of log messages (strings).

    Returns:
        tuple: A tuple containing the seed and PID as integers, or (None, None)
               if not found.
    """
    seed = None
    pid = None

    for message in log_messages:
        # Match for seed and PID in the first message
        match_seed_pid = re.search(r"seed: (\d+), and PID: (\d+)", message)
        if match_seed_pid:
            seed = int(match_seed_pid.group(1))
            pid = int(match_seed_pid.group(2))
            break  # Stop searching after finding the seed and PID

    return seed, pid

class EvolveRequest(BaseModel):
    environment: str

@router.post("/evolve", tags=["experiments"])
async def evolve(request: EvolveRequest):
    environment = request.environment
    cmd = ["tpg", "evolve", environment]

    try:
        output_lines, return_code = await execute_tpg(cmd)

        if return_code != 0:
            error_message = "\n".join(output_lines) or "Unknown error"
            raise HTTPException(
                status_code=400, detail=f"TPG evolve failed: {error_message}"
            )
        
        seed, pid = extract_seed_and_pid(output_lines)

        response_data = {"seed": seed, "pid": pid}

        return JSONResponse(content=response_data)
    except Exception as e:
        if isinstance(e, HTTPException):
            raise e  # Re-raise the HTTPException
        raise HTTPException(status_code=500, detail=str(e))

class ReplayRequest(BaseModel):
    environment: str
    seed: int

@router.post("/replay", tags=["experiments"])
async def replay(request: ReplayRequest):
    environment = request.environment
    seed = request.seed
    cmd = ["tpg", "replay", environment, "-s", str(seed)]

    try:
        output_lines, return_code = await execute_tpg(cmd)

        if return_code != 0:
            error_message = "\n".join(output_lines) or "Unknown error"
            raise HTTPException(
                status_code=400, detail=f"TPG replay failed: {error_message}"
            )

        response_data = {"status": "replay", "response": output_lines}

        return JSONResponse(content=response_data)
    except Exception as e:
        if isinstance(e, HTTPException):
            raise e  # Re-raise the HTTPException
        raise HTTPException(status_code=500, detail=str(e))

