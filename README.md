_From the product series "Magicodelogy"_

# Cellular Arcana

> **Note:** This project is **archived**. It was created during a study period where I experimented with cellular network devices. The code was written to interact with a specific hardware evaluation kit.

## 📖 About
**Cellular Arcana** is a simple network monitor application written in **C**. 

I built this tool to communicate with the **Quectel BG95** cellular module (specifically using the *277-UMTSLTEEVB-KIT-B* evaluation board). It interfaces with the device via serial (COM) ports to send AT commands and retrieve network status information.

## 🛠️ Hardware & Tech Stack
* **Language:** C
* **Hardware:** Quectel BG95 Module (LTE Cat M1/Cat NB2/EGPRS)
* **Dev Kit:** 277-UMTSLTEEVB-KIT-B
* **Communication:** Serial / UART (AT Commands)

## 📂 Project Structure
* `bg95_monitor.c`: The main source code handling the serial connection and AT command logic.
* `bg95_monitor.exe`: The compiled executable (Windows).

## 🚀 Usage
To use this tool, you typically need the evaluation board connected to your PC via USB.

1. **Connect the Board:** Ensure the Quectel EVB is connected and drivers are installed.
2. **Compile (Optional):**
   ```bash
   gcc bg95_monitor.c -o bg95_monitor
````

3.  **Run:**
    Execute the program to start monitoring the cellular connection.
    *(Note: You may need to modify the COM port in the source code to match your machine).*

## 📄 License

This project is open for educational and archival purposes.
