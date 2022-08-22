define cmdline
Please choose one of the specific commands:
  - build : build the docker image
  - once : probe once into the docker image and discard the changes
  - script : run a script over the docker image
endef
export cmdline

help:
	@echo "$$cmdline"

# path
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(mkfile_path))
project_dir := $(realpath $(current_dir)/../..)

where:
	@echo $(project_dir)
# variablee
tag = bf278a66aa1f

#targets
build:
	@docker build . -t $(tag)
once:
	@docker run --rm \
		-v $(project_dir):/project \
		-it \
		$(tag)

# When it is initialization we build the passes here
init:
	@docker run \
		-v $(project_dir):/project \
		-it \
		$(tag) \
		python3 /project/tests/docker/pass_build.py

test:
	@docker run \
		-v $(project_dir):/project \
		-it \
		$(tag) \
	        python3 /project/tests/mutation/evolution.py	
