import os
import sys
import pytest

EXPORTER_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOTTIE_DIR = os.path.join(os.path.dirname(EXPORTER_DIR), 'lottie-exporter')
sys.path.insert(0, EXPORTER_DIR)
sys.path.insert(0, LOTTIE_DIR)

@pytest.fixture
def fixtures_dir():
    return os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(EXPORTER_DIR))),
                        'synfig-core', 'test', 'fixtures')

@pytest.fixture
def exporter_dir():
    return EXPORTER_DIR
