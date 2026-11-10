# PROVENANCE-BEGIN: HUMAN-AUTHORED  Trace: T-003  DDR: DDR-001  req: SRS-001
def capture_health_check(sink_dir: str) -> dict:
    """Confirm the sink directory is writable."""
    import os, time
    probe = os.path.join(sink_dir, ".health_probe")
    try:
        open(probe, "w").write(str(time.time()))
        os.remove(probe)
        return {"sink_writable": True}
    except OSError as e:
        return {"sink_writable": False, "error": str(e)}
# PROVENANCE-END: HUMAN-AUTHORED