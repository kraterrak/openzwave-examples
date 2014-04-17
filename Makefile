.PHONY: clean

all:
	$(MAKE) -C ./ozw-power-on-off/
	$(MAKE) -C ./ozw-pir-active/

clean:
	$(MAKE) -C ./ozw-power-on-off/ $(MAKECMDGOALS)
	$(MAKE) -C ./ozw-pir-active/ $(MAKECMDGOALS)

