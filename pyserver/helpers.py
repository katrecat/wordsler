from enum import Enum


class MessageType(Enum):
    """
    Enum representing message type in the communication with the target server.

    For example, each message in the communication between the host and the
    target can start with 2 bytes unsigned integer representing the message
    type.

    OK - message indicating success of previous command
    ERROR - message indicating failure of previous command
    CONNECT - message indicating user connection
    DISCONNECT - message indicating user disconnect
    DATA - message indicating user data
    """
    OK = 0
    ERROR = 1
    CONNECT = 2
    DISCONNECT = 3
    DATA = 4

    def to_bytes(self) -> str:
        """
        Converts MessageType enum to bytes in uint16 format.

        Returns
        -------
        bytes : converted message type
        """
        return int(self.value).to_bytes(2, 'little', signed=False)
