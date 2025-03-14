from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware


from .routers import experiments


app = FastAPI()

origins = [
    "http://localhost:8080",
    "http://127.0.0.1:8080",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


app.include_router(experiments.router)

@app.get("/")
async def root():
    return {"message": "Hello Bigger Applications!"}