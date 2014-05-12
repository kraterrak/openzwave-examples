.PHONY: clean

MKDIR_P= mkdir -p

all:	meta build

build:
	$(MAKE) -C ./ozw-power-on-off/
	$(MAKE) -C ./ozw-pir-active/
	$(MAKE) -C ./ozw-pir-power-switch/
clean:
	$(MAKE) -C ./ozw-power-on-off/ $(MAKECMDGOALS)
	$(MAKE) -C ./ozw-pir-active/ $(MAKECMDGOALS)
	$(MAKE) -C ./ozw-pir-power-switch/ $(MAKECMDGOALS)
	$(info ***** To remove all zwave config files, run "rm -rf meta" *****)

meta:
	$(MKDIR_P) ./meta
