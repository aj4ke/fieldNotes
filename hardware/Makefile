ESPPORT ?= /dev/cu.usbmodem5CE51949831

IDF_ACTIVATE ?= $(HOME)/.espressif/tools/activate_idf_v6.0.2.sh
IDF_ENV = [ -n "$$IDF_PATH" ] || . $(IDF_ACTIVATE) >/dev/null;
IDF_PY = $(IDF_ENV) python "$$IDF_PATH/tools/idf.py"

.PHONY: build flash capture clean

build:
	@$(IDF_PY) build

flash:
	@$(IDF_PY) -p $(ESPPORT) flash

capture:
	@uv run tools/capture.py -p $(ESPPORT)

clean:
	@$(IDF_PY) fullclean
