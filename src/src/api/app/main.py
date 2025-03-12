from fastapi import FastAPI

from .routers import experiments


app = FastAPI()

app.include_router(experiments.router)

@app.get("/")
async def root():
    return {"message": "Hello Bigger Applications!"}