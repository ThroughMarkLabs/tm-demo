# SAFELOG — safety-critical logging library
#
# This module provides LogEntry and Logger. File/network I/O, append-mode
# writes, fsync durability, config wiring, and logging scaffolding are
# standard, first-principles plumbing — fenced BOILERPLATE.
#
# NOTE: the SRS-002 null/empty input validation is domain-specific invention.
# That logic is intentionally left as a HUMAN-AUTHORED placeholder and must be
# filled and recorded by the developer before this module is merged.

# PROVENANCE-BEGIN: BOILERPLATE  Agent: gemma  Trace: d66cf2f11f58a352  Module: safelog
# Date: 2026-11-04
import os
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import IO

# PROVENANCE-END: BOILERPLATE

# PROVENANCE-BEGIN: BOILERPLATE  Agent: gemma  Trace: d66cf2f11f58a352  Module: safelog
# Date: 2026-11-04
@dataclass(frozen=True)
class LogEntry:
    """An immutable log entry with a UTC ISO-8601 timestamp."""

    timestamp: str
    level: str
    message: str

    def to_line(self) -> str:
        """Serialize the entry to a single log line."""
        return f"{self.timestamp} | {self.level} | {self.message}\n"

    @classmethod
    def create(cls, level: str, message: str) -> "LogEntry":
        """Build a LogEntry stamped with the current UTC time."""
        timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%fZ")
        return cls(timestamp=timestamp, level=level, message=message)


class Logger:
    """Appends LogEntry records to a file with fsync durability."""

    def __init__(self, path: str) -> None:
        self._path = path
        self._handle: IO[str] = open(path, "a", encoding="utf-8")

    def log(self, entry: LogEntry) -> None:
        """Write a single entry to the log and make it durable."""
        self._handle.write(entry.to_line())
        self._handle.flush()
        os.fsync(self._handle.fileno())

    def log_line(self, level: str, message: str) -> None:
        """Convenience: build an entry from a level+message and write it."""
        self.log(LogEntry.create(level, message))

    def close(self) -> None:
        """Flush and close the underlying file handle."""
        if not self._handle.closed:
            self._handle.flush()
            os.fsync(self._handle.fileno())
            self._handle.close()

    def __enter__(self) -> "Logger":
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> bool:
        self.close()
        return False

# PROVENANCE-END: BOILERPLATE

# PROVENANCE-BEGIN: HUMAN-AUTHORED  Developer: <name>  Trace: d66cf2f11f58a352  Module: safelog
# DDR: docs/ddr/DDR-<id>.md
# TODO(human): Implement the null/empty message rejection for SRS-002.
#   Define the validation contract here (raise ValueError on None/empty/whitespace?),
#   where it fires (LogEntry.create vs Logger.log), and record the decision in a DDR.
# PROVENANCE-END: HUMAN-AUTHORED
