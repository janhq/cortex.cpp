import json
import os

def log_response(data, test_name):
    """Log the data to a file named after the test."""
    log_dir="e2e-test/logs"
    os.makedirs(log_dir, exist_ok=True)  # Ensure log directory exists
    file_path = os.path.join(log_dir, f"{test_name}.txt")  # Log file per test

    try:
        with open(file_path, "a", encoding="utf-8") as file:
            json.dump(data, file, indent=4)
            file.write("\n")  # Ensure a new line between entries
    except Exception as e:
        print(f"Error logging response: {e}")
