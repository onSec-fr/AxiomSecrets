CORE_PATH = ./src/
CORE =	main.cpp	\
	strdup.cpp	\
	strsplit.cpp

CORE_SRCS = $(addprefix $(CORE_PATH),$(CORE))
SRCS += $(CORE_SRCS)

