CONTIKI_PROJECT=wf_contiki
all: $(CONTIKI_PROJECT)

CONTIKI=../..

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += wfradio_driver.c

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
