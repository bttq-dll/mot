# KCD Mod Order Tool

A simple Windows application for managing and creating mod orders for *Kingdom Come: Deliverance* using the mod folders in your directory.

## Features

- Displays a list of mod folders
- Renames folders that contain spaces or underscores
- Allows you to create a `mod_order.txt` file with the list of mods
- Custom UI with buttons for refreshing mods and creating the mod order file

## Requirements

- **Windows**: The application requires Windows 7 or later.
- **Visual Studio**: This project was developed using Visual Studio 2022 with the **v143 toolset** and **C++20** standard (`/std:c++20`).
- **NuGet Packages**: The project uses **Common Controls** (`comctl32.lib`) and **SHLWAPI** (`shlwapi.lib`).

## Getting Started

Since this project doesn't include a `.sln` or `.vcxproj` file, you will need to create the necessary project files yourself in Visual Studio. Here's how to get started:

### 1. Clone the Repository

First, clone the repository:

```bash
git clone https://github.com/bttq-dll/mot.git
cd mot
```

### 2. Create a New Project in Visual Studio

1. Open **Visual Studio 2022** (or higher).
2. Create a **New Project** and choose **Empty Project** under **C++**.
3. Name the project `MOT1.1` and set the location to the folder where you cloned the repository.
4. Click **Create**.

### 3. Add Source Files

1. In **Solution Explorer**, right-click the **Source Files** folder and choose **Add > Existing Item**.
2. Select all the `.cpp` files from the cloned repository (including the ones with the source code for the app).
3. Add the `resource.h`, `MOT1.1.h`, and other necessary header files to the **Header Files** folder in the same way.

### 4. Configure Project Settings

- **Set the C++ Standard**: Go to **Project** > **Properties** > **C/C++** > **Language** and set the **C++ Language Standard** to **C++20** (`/std:c++20`).
  
- **Set the Toolset**: Ensure you're using the **v143 toolset** by going to **Project** > **Properties** > **General** and setting **Platform Toolset** to **Visual Studio 2022 (v143)**.

- **Linking Dependencies**:
    - Go to **Project** > **Properties** > **Linker** > **Input** and add the following libraries to **Additional Dependencies**:
      - `comctl32.lib`
      - `shlwapi.lib`

- **Set the Character Set**: Ensure **Use Multi-Byte Character Set** is selected (under **Project** > **Properties** > **General**).

### 5. Build the Project

To build the project:

1. Open **Build** > **Build Solution** or press `Ctrl + Shift + B`.
2. Once the build process is complete, you will find the executable (`KCDModManager.exe`) in the `x64\Release` (or `x86\Release`) folder, depending on the platform target.

### 6. Running the Application

After building, move to application to mods folder, you can run the application by double-clicking `MOT1.1.exe`.

## Usage

Once the application is running:

1. Click on **Refresh mods** to load the mod folders from the current directory.
2. If any mod folder names contain spaces or underscores, you will be prompted to rename them.
3. After the mods are loaded, you can click **Create mod_order.txt** to generate a text file with the list of all mod folders.

You can also access the Nexus page by clicking on the **by itsbttq for nexusmods** link.

