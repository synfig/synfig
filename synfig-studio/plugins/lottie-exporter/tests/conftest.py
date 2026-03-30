import os
import sys
import pytest

# Add the lottie-exporter directory to the Python path
EXPORTER_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, EXPORTER_DIR)


@pytest.fixture
def fixtures_dir():
    """Return the path to test fixtures directory."""
    return os.path.join(os.path.dirname(__file__), "fixtures")


@pytest.fixture
def exporter_dir():
    """Return the path to the lottie-exporter plugin."""
    return EXPORTER_DIR
