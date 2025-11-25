import serial
import time
from typing import Optional
from enum import Enum


class DotDict(dict):
    """Dictionary subclass that allows dot notation access to keys."""
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # Create reverse mapping: name -> id
        self._name_to_id = {name: id for id, name in self.items()}
    
    def __getattr__(self, name):
        """Allow dot notation access: vars.temp returns the ID"""
        if name.startswith('_'):
            return super().__getattribute__(name)
        try:
            return self._name_to_id[name]
        except KeyError:
            raise AttributeError(f"Variable '{name}' not found")
    
    def __setattr__(self, name, value):
        """Prevent setting attributes via dot notation"""
        if name.startswith('_'):
            super().__setattr__(name, value)
        else:
            raise AttributeError("Cannot modify variables dictionary. Use client.set_variable() instead")
    
    def __dir__(self):
        """Make variable names show up in tab completion"""
        return list(self._name_to_id.keys()) + list(super().__dir__())
    
    def __repr__(self):
        items = ''.join(f"{name}: {id}\n" for id, name in sorted(self.items()))
        return f"Variables(\n{items})"


class TelemetryCommand(Enum):
    """Enumeration of available telemetry commands"""
    GET_VARIABLE_NAMES = "get_variable_names"
    GET_VARIABLE = "get_variable"
    SET_VARIABLE = "set_variable"


class TelemetryUARTClient:
    """
    UART client for communicating with the telemetry system.
    
    Example usage:
        client = TelemetryUARTClient('/dev/ttyUSB0', baudrate=115200)
        client.connect()
        response = client.send_command(TelemetryCommand.GET_TEMP)
        print(response)
        client.disconnect()
    """
    
    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1.0):
        """
        Initialize the UART client.
        
        Args:
            port: Serial port path (e.g., '/dev/ttyUSB0' or 'COM3')
            baudrate: Communication baudrate (default: 115200)
            timeout: Read timeout in seconds (default: 1.0)
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial_conn: Optional[serial.Serial] = None
        if not self.connect():
            raise ConnectionError(f"Failed to connect to {self.port} at {self.baudrate} baud")
        self.variables = self.get_variable_names()
        
    def connect(self) -> bool:
        """
        Open the serial connection.
        
        Returns:
            True if connection successful, False otherwise
        """
        try:
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            time.sleep(0.1)  # Allow time for connection to stabilize
            print(f"Connected to {self.port} at {self.baudrate} baud")
            return True
        except serial.SerialException as e:
            print(f"Failed to connect: {e}")
            return False
            
    def disconnect(self):
        """Close the serial connection."""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            print(f"Disconnected from {self.port}")
            
    def is_connected(self) -> bool:
        """Check if serial connection is open."""
        return self.serial_conn is not None and self.serial_conn.is_open
        
    def send_command(self, command: TelemetryCommand, args: str = "") -> str:
        """
        Send a command to the telemetry system.
        
        Args:
            command: The command to send (from TelemetryCommand enum)
            args: Arguments for the command (will be wrapped in parentheses)
            
        Returns:
            Response string from the device
            
        Raises:
            ConnectionError: If not connected to serial port
        """
        if not self.is_connected():
            raise ConnectionError("Not connected to serial port. Call connect() first.")
            
        # Build command string with parentheses format: command_name(args)
        cmd_str = f"{command.value}({args})\n"
            
        try:
            # Clear input buffer
            self.serial_conn.reset_input_buffer()
            
            # Send command
            self.serial_conn.write(cmd_str.encode('utf-8'))
            self.serial_conn.flush()
            
            # Read response
            response = self._read_response()
            return response
            
        except serial.SerialException as e:
            print(f"Communication error: {e}")
            return ""
            
    def send_raw_command(self, command_str: str) -> str:
        """
        Send a raw command string to the device.
        
        Args:
            command_str: Raw command string (newline will be added automatically)
            
        Returns:
            Response string from the device
        """
        if not self.is_connected():
            raise ConnectionError("Not connected to serial port. Call connect() first.")
            
        try:
            # Add newline if not present
            if not command_str.endswith('\n'):
                command_str += '\n'
                
            self.serial_conn.reset_input_buffer()
            self.serial_conn.write(command_str.encode('utf-8'))
            self.serial_conn.flush()
            
            response = self._read_response()
            return response
            
        except serial.SerialException as e:
            print(f"Communication error: {e}")
            return ""
            
    def _read_response(self, max_lines: int = 10) -> str:
        """
        Read response from the device.
        
        Args:
            max_lines: Maximum number of lines to read
            
        Returns:
            Complete response as a string
        """
        response_lines = []
        
        for _ in range(max_lines):
            try:
                line = self.serial_conn.readline().decode('utf-8').strip()
                if line:
                    response_lines.append(line)
                else:
                    break  # Empty line, end of response
            except UnicodeDecodeError:
                continue
                
        return '\n'.join(response_lines)
        
    def get_variable_names(self) -> DotDict:
        """
        Get all variable names and their IDs.
        
        Returns:
            Dictionary mapping variable IDs to names
        """
        response = self.send_command(TelemetryCommand.GET_VARIABLE_NAMES)
        
        variables = {}
        if response:
            # Parse format: "0:temp;1:humidity;2:pressure;"
            pairs = response.split(';')
            for pair in pairs:
                if ':' in pair:
                    id_str, name = pair.split(':', 1)
                    try:
                        variables[int(id_str)] = name.upper()
                    except ValueError:
                        continue
        return DotDict(variables)
    
    def get_variable(self, var_id: int) -> tuple:
        """
        Get a specific variable value by ID.
        
        Args:
            var_id: Variable ID to query
            
        Returns:
            Tuple of (name, value) or (None, None) if error
        """
        response = self.send_command(TelemetryCommand.GET_VARIABLE, str(var_id))
        
        if response.startswith("OK;"):
            # Parse format: "OK;temp:25.3"
            data = response[3:].strip()
            if ':' in data:
                name, value_str = data.split(':', 1)
                try:
                    value = float(value_str)
                    return (name, value)
                except ValueError:
                    pass
        return (None, None)
    
    def set_variable(self, var_id: int, value: float) -> bool:
        """
        Set a single variable value.
        
        Args:
            var_id: Variable ID to set
            value: New value
            
        Returns:
            True if successful, False otherwise
        """
        args = f"{var_id}:{value}"
        response = self.send_command(TelemetryCommand.SET_VARIABLE, args)
        return response.strip() == "OK"
    
    def set_variables(self, variables_to_set: list, values_to_set: list) -> bool:
        """
        Set multiple variables at once.
        
        Args:
            variables_to_set: List of variable IDs
            values_to_set: List of corresponding values
            
        Returns:
            True if successful, False otherwise
        """
        if len(variables_to_set) != len(values_to_set):
            raise ValueError("variables_to_set and values_to_set must have the same length")

        # Build format: "0:25.5;1:60.2;2:1013.25"
        args = ';'.join(f"{var_id}:{value}" 
                        for var_id, value in zip(variables_to_set, values_to_set))

        response = self.send_command(TelemetryCommand.SET_VARIABLE, args)
        return response.strip() == "OK"
                
    def __enter__(self):
        """Context manager entry."""
        self.connect()
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self.disconnect()
