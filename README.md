
# ATM System Simulation

## Introduction

This project simulates an ATM system using concurrent processes and Inter-Process Communication (IPC) mechanisms such as shared memory, semaphores, and message queues. The system consists of three main components:

- **ATM Process**: Handles user interactions such as account number, PIN input, and banking operations.
- **DB Server Process**: Manages the backend database, including account verification and transaction processing.
- **DB Editor Process**: Allows updating account information or adding new accounts.

The system demonstrates how concurrency and IPC mechanisms can be used to build a distributed, multi-process application.

## Files Overview

- **ATM.c**: Source code for the ATM process.
- **DBserver.c**: Source code for the DB server.
- **DBeditor.c**: Source code for the DB editor.
- **DataBase.csv**: Initial database file containing account information.
- **key_file.txt**: Semaphore key file used for synchronization.
- **Makefile**: Used for compiling the project.

## Requirements

To run this project, you need:

- **Linux environment** (Ubuntu recommended).
- **Windows Subsystem for Linux (WSL)** if running on Windows.
- **GCC** compiler to compile the C programs.
- **Make utility** to automate the build process.

## Setup

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/SajaFawagreh/ATM-System-Simulation.git
   ```

2. **Install WSL (Windows Only)**:
   If you are running this on Windows, you need to install WSL.

3. **Navigate to the Project Directory**:
   Open a terminal (or WSL terminal for Windows users) and navigate to the extracted project directory.

   ```bash
   cd /path/to/ATM-System-Simulation
   ```

4. **Compile the Project**:
   Use the `make` command to compile all the necessary files:

   ```bash
   make
   ```

   This will generate the executables for the ATM, DB Server, and DB Editor.

## Operation Manual

### Step 1: Start the System
- After compiling, open three terminal windows to run the three main processes (DB Server, DB Editor, and ATM).

1. **Start the ATM**:
   In the first terminal, run the ATM process to simulate customer interactions:

   ```bash
   ./ATM
   ```

   The ATM will prompt for account number and PIN input.

2. **Start the DB Server**:
   In the second terminal, run the DB Server process to manage the account data:

   ```bash
   ./DBserver
   ```

   The DB Server will start and listen for requests from the ATM and DB Editor.

3. **Start the DB Editor**:
   In the third terminal, run the DB Editor process to manage the database:

   ```bash
   ./DBeditor
   ```

   The DB Editor allows you to update or create new accounts.

### Step 2: ATM Operation
1. **Enter the Account Number**:
   - When prompted, enter the account number (e.g., `00001`).
   - If the account number is not 5 digits, you will be prompted to re-enter.

2. **Enter the PIN**:
   - The system will ask for the PIN. Enter the 3-digit PIN associated with the account (e.g., `107`).
   - If the PIN is incorrect, the system will notify you. After three incorrect attempts, the account will be locked.

3. **Choose an Operation**:
   - Once the PIN is verified, you will be asked to choose between two operations:
     - **Balance Inquiry**: Retrieve and display the current balance.
     - **Withdrawal**: Withdraw funds from the account.

4. **Perform the Operation**:
   - **Balance Inquiry**: The ATM will display the current balance after retrieving it from the DB Server.
   - **Withdrawal**: The ATM will prompt you to enter the withdrawal amount. If sufficient funds are available, the DB Server will deduct the amount and update the balance. If not, it will display "Insufficient Funds".

5. **End the ATM Process**:
   - To end the ATM process, type `X` when prompted for the account number.

### Step 3: DB Editor Operation
1. **Enter Account Details**:
   - The DB Editor will ask for:
     - **Account Number** (5 digits).
     - **PIN** (3 digits).
     - **Funds** (Initial balance in the account).

2. **Update/Create Account**:
   - The DB Editor sends the account information to the DB Server. If the account exists, it updates the details; otherwise, it creates a new account.

3. **End the DB Editor Process**:
   - Type `X` when prompted for the account number to terminate the DB Editor process.

### State Diagram
For a detailed understanding of the workflow and how the system works, refer to the [State Diagram](https://github.com/SajaFawagreh/ATM-System-Simulation/blob/233c82fd88ddceb81602acd92113ba0fcc48cbe1/State%20Diagram.png) included in this repository. The diagram provides a step-by-step representation of the interactions between the ATM, DB Server, and DB Editor, including conditions for valid account numbers, PIN verification, and transaction processing.

## License
This project is for educational purposes and part of the SYSC 4001 course. No formal license is provided.
