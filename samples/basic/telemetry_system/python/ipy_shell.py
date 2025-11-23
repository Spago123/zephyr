#!/usr/bin/env python3
"""
Interactive IPython shell for telemetry system communication.

Usage:
    python interactive_shell.py [--port PORT] [--baudrate BAUDRATE]
    python interactive_shell.py --interactive
    
Examples:
    python interactive_shell.py --port /dev/ttyUSB0 --baudrate 115200
    python interactive_shell.py -i
"""

import sys
import argparse
from telemetry import TelemetryUARTClient, TelemetryCommand


def get_interactive_config():
    """Interactively prompt user for connection parameters."""
    print("\n" + "="*70)
    print("  Telemetry System - Interactive Configuration")
    print("="*70)
    
    # Get port
    print("\nAvailable serial ports (common examples):")
    print("  Linux:   /dev/ttyUSB0, /dev/ttyACM0, /dev/ttyS0")
    print("  Windows: COM1, COM3, COM4")
    print("  macOS:   /dev/cu.usbserial, /dev/tty.usbserial")
    
    while True:
        port = input("\nEnter serial port [default: /dev/ttyUSB0]: ").strip()
        if not port:
            port = '/dev/ttyUSB0'
        
        if port:
            break
        print("Port cannot be empty!")
    
    # Get baudrate
    print("\nCommon baudrates: 9600, 19200, 38400, 57600, 115200, 230400")
    
    while True:
        baudrate_input = input("Enter baudrate [default: 115200]: ").strip()
        if not baudrate_input:
            baudrate = 115200
            break
        
        try:
            baudrate = int(baudrate_input)
            if baudrate > 0:
                break
            print("Baudrate must be positive!")
        except ValueError:
            print("Invalid baudrate! Please enter a number.")
    
    print("\n" + "="*70)
    print(f"  Configuration: {port} @ {baudrate} baud")
    print("="*70 + "\n")
    
    return port, baudrate


def list_available_ports():
    """List available serial ports if pyserial is available."""
    try:
        import serial.tools.list_ports
        ports = serial.tools.list_ports.comports()
        
        if ports:
            print("\nDetected serial ports:")
            for port in ports:
                print(f"  - {port.device}")
                if port.description:
                    print(f"    Description: {port.description}")
        else:
            print("\nNo serial ports detected.")
    except ImportError:
        pass


def main():
    parser = argparse.ArgumentParser(
        description='Interactive shell for telemetry system communication',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --port /dev/ttyUSB0 --baudrate 115200
  %(prog)s -p COM3 -b 9600
  %(prog)s --interactive
  %(prog)s -i
        """
    )
    
    parser.add_argument(
        '-p', '--port',
        type=str,
        help='Serial port (e.g., /dev/ttyUSB0, COM3)'
    )
    
    parser.add_argument(
        '-b', '--baudrate',
        type=int,
        help='Baudrate (e.g., 9600, 115200)'
    )
    
    parser.add_argument(
        '-i', '--interactive',
        action='store_true',
        help='Interactive mode - prompt for port and baudrate'
    )
    
    parser.add_argument(
        '-l', '--list-ports',
        action='store_true',
        help='List available serial ports and exit'
    )
    
    parser.add_argument(
        '-t', '--timeout',
        type=float,
        default=1.0,
        help='Serial timeout in seconds (default: 1.0)'
    )
    
    args = parser.parse_args()
    
    # Handle --list-ports
    if args.list_ports:
        list_available_ports()
        sys.exit(0)
    
    # Determine port and baudrate
    if args.interactive:
        port, baudrate = get_interactive_config()
    else:
        # Use command line args or defaults
        port = args.port if args.port else '/dev/ttyUSB0'
        baudrate = args.baudrate if args.baudrate else 115200
        
        if not args.port or not args.baudrate:
            print(f"Using defaults: {port} @ {baudrate} baud")
            print("(Use --interactive or -i for interactive configuration)\n")
    
    # Create and connect client
    print(f"Connecting to {port} @ {baudrate} baud...")
    client = TelemetryUARTClient(port, baudrate, timeout=args.timeout)
    
    if not client.connect():
        print("\n❌ Failed to connect. Please check:")
        print("  - Port name is correct")
        print("  - Device is connected")
        print("  - You have permission to access the port")
        print("  - Baudrate matches device configuration")
        print("\nTip: Use --list-ports to see available ports")
        sys.exit(1)
    
    print("✓ Connected successfully!\n")
    
    # Print welcome message
    print("="*70)
    print("  Telemetry System Interactive Shell")
    print("="*70)
    print(f"  Port:     {port}")
    print(f"  Baudrate: {baudrate}")
    print(f"  Timeout:  {args.timeout}s")
    print("\n  Available objects:")
    print("    • client          - TelemetryUARTClient instance")
    print("    • TelemetryCommand - Command enum")
    print("\n  Tips:")
    print("    • Tab completion is available")
    print("    • Use 'client?' for help")
    print("    • Type 'exit()' or Ctrl+D to quit")
    print("="*70 + "\n")
    
    # Start IPython shell
    try:
        from IPython import embed
        embed(colors='neutral', banner1='')
    except ImportError:
        print("⚠ IPython not installed!")
        print("Install with: pip install ipython")
        print("\nFalling back to standard Python shell...\n")
        
        import code
        code.interact(local=locals(), banner='')
    finally:
        print("\nDisconnecting...")
        client.disconnect()
        print("Goodbye! 👋")


if __name__ == "__main__":
    main()