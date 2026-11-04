import runpy
from pathlib import Path


if __name__ == "__main__":
    comparator = Path(__file__).parent / "tools" / "compare_vti_outputs.py"
    runpy.run_path(str(comparator), run_name="__main__")
