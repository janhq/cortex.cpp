import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiGetHardware:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_hardware_successfully(self):
        hardware_url = f"http://localhost:3928/v1/hardware"
        hardware_response = requests.get(hardware_url)
        json_data_hardware = hardware_response.json()
        log_response(json_data_hardware, "test_api_get_hardware_successfully")
        assert_equal(hardware_response.status_code,200)
        
        schema = {
            "type": "object",
            "properties": {
            "cpu": {
                "type": "object",
                "properties": {
                "arch": {
                    "type": "string",
                    "example": "amd64",
                    "description": "The architecture of the CPU."
                },
                "cores": {
                    "type": "integer",
                    "example": 8,
                    "description": "The number of CPU cores available."
                },
                "instructions": {
                    "type": "array",
                    "items": {
                    "type": "string"
                    },
                    "example": [
                    "fpu",
                    "mmx",
                    "sse",
                    "sse2",
                    "sse3",
                    "ssse3",
                    "sse4_1",
                    "sse4_2",
                    "pclmulqdq",
                    "avx",
                    "avx2",
                    "aes",
                    "f16c"
                    ],
                    "description": "A list of supported CPU instruction sets."
                },
                "model": {
                    "type": "string",
                    "example": "AMD Ryzen Threadripper PRO 5955WX 16-Cores",
                    "description": "The model name of the CPU."
                }
                },
                "required": [
                "arch",
                "cores",
                "instructions",
                "model"
                ]
            },
            "gpus": {
                "type": "array",
                "items": {
                "type": "object",
                "properties": {
                    "activated": {
                    "type": "boolean",
                    "example": True,
                    "description": "Indicates if the GPU is currently activated."
                    },
                    "additional_information": {
                    "type": "object",
                    "properties": {
                        "compute_cap": {
                        "type": "string",
                        "example": "8.6",
                        "description": "The compute capability of the GPU."
                        },
                        "driver_version": {
                        "type": "string",
                        "example": "535.183",
                        "description": "The version of the installed driver."
                        }
                    },
                    "required": [
                        "compute_cap",
                        "driver_version"
                    ]
                    },
                    "free_vram": {
                    "type": "integer",
                    "example": 23983,
                    "description": "The amount of free VRAM in MB."
                    },
                    "id": {
                    "type": "string",
                    "example": "0",
                    "description": "Unique identifier for the GPU."
                    },
                    "name": {
                    "type": "string",
                    "example": "NVIDIA GeForce RTX 3090",
                    "description": "The name of the GPU model."
                    },
                    "total_vram": {
                    "type": "integer",
                    "example": 24576,
                    "description": "The total VRAM available in MB."
                    },
                    "uuid": {
                    "type": "string",
                    "example": "GPU-5206045b-2a1c-1e7d-6c60-d7c367d02376",
                    "description": "The universally unique identifier for the GPU."
                    },
                    "version": {
                    "type": "string",
                    "example": "12.2",
                    "description": "The version of the GPU."
                    }
                },
                "required": [
                    "activated",
                    "additional_information",
                    "free_vram",
                    "id",
                    "name",
                    "total_vram",
                    "uuid",
                    "version"
                ]
                }
            },
            "os": {
                "type": "object",
                "properties": {
                "name": {
                    "type": "string",
                    "example": "Ubuntu 24.04.1 LTS",
                    "description": "The name of the operating system."
                },
                "version": {
                    "type": "string",
                    "example": "24.04.1 LTS (Noble Numbat)",
                    "description": "The version of the operating system."
                }
                },
                "required": [
                "name",
                "version"
                ]
            },
            "power": {
                "type": "object",
                "properties": {
                "battery_life": {
                    "type": "integer",
                    "example": 0,
                    "description": "The percentage of battery life remaining."
                },
                "charging_status": {
                    "type": "string",
                    "example": "",
                    "description": "The charging status of the device."
                },
                "is_power_saving": {
                    "type": "boolean",
                    "example": False,
                    "description": "Indicates if the power-saving mode is enabled."
                }
                },
                "required": [
                "battery_life",
                "charging_status",
                "is_power_saving"
                ]
            },
            "ram": {
                "type": "object",
                "properties": {
                "available": {
                    "type": "integer",
                    "example": 11100,
                    "description": "The amount of available RAM in MB."
                },
                "total": {
                    "type": "integer",
                    "example": 15991,
                    "description": "The total RAM in MB."
                },
                "type": {
                    "type": "string",
                    "example": "",
                    "description": "The type of RAM."
                }
                },
                "required": [
                "available",
                "total",
                "type"
                ]
            },
            "storage": {
                "type": "object",
                "properties": {
                "available": {
                    "type": "integer",
                    "example": 0,
                    "description": "The amount of available storage in MB."
                },
                "total": {
                    "type": "integer",
                    "example": 0,
                    "description": "The total storage in MB."
                },
                "type": {
                    "type": "string",
                    "example": "",
                    "description": "The type of storage."
                }
                },
                "required": [
                "available",
                "total",
                "type"
                ]
            }
            }
        }

        # Validate response schema
        jsonschema.validate(instance=json_data_hardware, schema=schema)