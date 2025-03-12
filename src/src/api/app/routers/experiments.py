import subprocess
import asyncio

from fastapi import APIRouter, HTTPException
from fastapi.responses import JSONResponse

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


@router.post("/evolve", tags=["experiments"])
async def evolve():
    cmd = ["tpg", "evolve", "inverted_pendulum"]

    try:
        output_lines, return_code = await execute_tpg(cmd)

        if return_code != 0:
            error_message = "\n".join(output_lines) or "Unknown error"
            raise HTTPException(
                status_code=400, detail=f"TPG evolve failed: {error_message}"
            )

        response_data = {"status": "evolve", "response": output_lines}

        return JSONResponse(content=response_data)
    except Exception as e:
        if isinstance(e, HTTPException):
            raise e  # Re-raise the HTTPException
        raise HTTPException(status_code=500, detail=str(e))



@router.post("/replay", tags=["experiments"])
async def replay():
    cmd = ["tpg", "replay", "inverted_pendulum"]

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

