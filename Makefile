### Runs make in the src file and moves the executible to the main folder

.PHONY: all
.PHONY: clean

all:
	$(MAKE) -C src/
	mv src/BotnetBot .
	mv src/BotnetServer .

clean:
	$(MAKE) -C src/ clean
