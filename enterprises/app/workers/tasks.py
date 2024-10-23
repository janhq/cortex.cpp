from app.workers.celery_ import get_celery
import docker
import logging
import asyncio
import aiohttp
import json
from app.logger import get_task_logger
from app.config import get_config
from app.models.chat_completion import *
from app.utils import decode_base64_to_audio, encode_audio_to_base64

logger = get_task_logger()

_celery = get_celery()


async def CreateChatCompletion(request: dict):
    async with aiohttp.ClientSession() as session:
        async with session.post(get_config().llm_endpoint+"/v1/chat/completions", json=request) as response:
            result = await response.json()
    return result


async def CreateChatCompletionAudio(request: ChatCompletionRequest):
    for message in request.messages:
        if message.role == "user":
            content = ""
            if type(message.content) != list:
                continue
            for obj in message.content:
                if obj.type == "input_audio":
                    byte = decode_base64_to_audio(obj.input_audio.data)
                    audio_format = obj.input_audio.format
                    data = aiohttp.FormData()
                    data.add_field('file',
                                   byte,
                                   filename=f"file{AUDIO_FILE_EXTENSIONS[audio_format]}",
                                   content_type=AUDIO_MIME_TYPES[audio_format])
                    async with aiohttp.ClientSession() as session:
                        async with session.post(get_config().whisper_endpoint+f"/tokenize/{audio_format}", data=data) as response:
                            res = await response.json()

                    content += res["tokens"]
                else:
                    content += obj.text
            message.content = content
        elif message.role == "assistant":
            if message.audio is not None:
                message.content = "<|sound_token|>"

    async with aiohttp.ClientSession() as session:
        async with session.post(get_config().llm_endpoint+"/v1/chat/completions", json=request.dict()) as response:
            result = await response.json()
    return result


@_celery.task(bind=True, queue=get_config().celery_job_queue)
def run_completion_task(self, request: dict):
    task_id = self.request.id
    logger.info(f"Starting task {task_id}")

    try:
        # Pull the Docker image
        loop = asyncio.get_event_loop()
        out = loop.run_until_complete(
            CreateChatCompletion(request))
        logger.info(f"Task {task_id} completed successfully")
        return out

    except Exception as e:
        logger.error(f"Error running task: {str(e)}")
        raise


@_celery.task(bind=True, queue=get_config().celery_job_queue)
def run_completion_audio_task(self, request: dict):
    task_id = self.request.id
    logger.info(f"Starting task {task_id}")

    try:
        # Pull the Docker image
        loop = asyncio.get_event_loop()
        out = loop.run_until_complete(
            CreateChatCompletionAudio(ChatCompletionRequest.model_validate(request)))
        logger.info(f"Task {task_id} completed successfully")
        return out

    except Exception as e:
        logger.error(f"Error running task: {str(e)}")
        raise
