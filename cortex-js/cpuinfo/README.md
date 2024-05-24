# CPU Info

This source code provides a simple C++ program to retrieve and display information about the CPU, including the vendor, brand, number of cores, number of logical processors (threads), and supported instruction sets (e.g., SSE, AVX, AVX512).

## Files

### CPUID.h

This header file contains the CPUID class, which abstracts the CPUID instruction. The class provides methods to retrieve the values of the EAX, EBX, ECX, and EDX registers after calling the CPUID instruction with a given function ID.

### cpuinfo.cpp

This source file implements the CPUInfo class, which uses the CPUID class to gather various CPU-related information. The main function prints this information in JSON format.

## Building

### Windows

To build the project on Windows, you can use Microsoft Visual Studio or the Visual Studio Developer Command Prompt.

**Using Visual Studio Developer Command Prompt**

1. Open the Developer Command Prompt for Visual Studio.

2. Navigate to the project directory:
    ```cmd
        cd path\to\your\project\directory
    ```

3. Compile the source code using the following command:
    ```cmd
        cl cpuinfo.cpp /EHsc
    ```

    This will create the executable cpuinfo.exe in the current directory.

### Linux

To build the project on Linux, you need g++ (the GNU C++ compiler).

1. Open a terminal.

2. Navigate to the project directory:
    ```bash
        cd path/to/your/project/directory
    ```

3. Compile the code:
    ```bash
        g++ cpuinfo.cpp -o cpuinfo
    ```

    This will create the executable cpuinfo in the current directory.

## Running the Program

### Windows

After building the project, you can run the executable from the command prompt:

```cmd
    cpuinfo.exe
```

### Linux

After building the project, you can run the executable from the terminal:

```bash
    ./cpuinfo
```

## Example Output

The program prints the CPU information in JSON format. Example output:

```json
{
  "vendor": "GenuineIntel",
  "brand": "12th Gen Intel(R) Core(TM) i5-12400F",
  "cores": 6,
  "threads": 12,
  "is_hyperthreading": true,
  "instructions": {
    "SSE": true,
    "SSE2": true,
    "SSE3": true,
    "SSE41": true,
    "SSE42": true,
    "AVX": true,
    "AVX2": true,
    "AVX512": true
  }
}
```

## Notes
- Ensure that your environment is properly set up for compiling C++ code. On Windows, this typically involves installing Visual Studio with the C++ build tools. On Linux, you need to have g++ installed.
- The JSON output format is designed to be easy to parse and read.
- In the `bin` directory, there are pre-built outputs. For Windows, the code is signed with our certificate. You can download and use it directly or build from source.