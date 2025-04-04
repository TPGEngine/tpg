from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware


from .routers import experiments, signaling


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
app.include_router(signaling.router, prefix="/ws", tags=["signalling"])

@app.get("/")
async def root():
    return {"message": "Hello Bigger Applications!"}