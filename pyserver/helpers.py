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
    USERNAME - message indicating user setting username
    """
    OK = 0
    ERROR = 1
    CONNECT = 2
    DISCONNECT = 3
    DATA = 4
    USERNAME = 5

    def to_bytes(self) -> str:
        """
        Converts MessageType enum to bytes in uint16 format.

        Returns
        -------
        bytes : converted message type
        """
        return int(self.value).to_bytes(2, 'little', signed=False)

    def from_bytes(value: bytes) -> 'MessageType':
        """
        Converts 2-byte bytes to MessageType enum.

        Parameters
        ----------
        value: bytes
            enum in bytes

        Returns
        -------
        MessageType : enum value
        """
        return MessageType(int.from_bytes(value, 'little', signed=False))


def wordFromBytes(word: bytes) -> str:
    return word.decode('utf-8')
