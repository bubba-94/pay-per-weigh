.PHONY: docs docs-clean regen-docs

# Generate documentation and open Chrome
docs:
	doxygen Doxyfile

# Clean documentation
docs-clean:
	rm -rf docs/html docs/latex

# Clean + regenerate in one command
regen-docs: docs-clean docs