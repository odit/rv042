JAVA	=	java
JAR	=	jar
JAVAC	=	javac
#DEBUG	=	-g
JFLAGS	=	-classpath $(CLASSPATH):.
SRCDIR	=	de org
PKGNAME	=	WebTelnet
VERSION	=	`java -version 2>&1 | head -1 | \
		 sed 's/^java version //' | sed 's/"//g'`
DATE	=	`date +%Y%m%d-%H%M`

.SUFFIXES:	.java .class

# compile java files to class
.java.class:
	$(JAVAC) $(DEBUG) $(JFLAGS) $<

#
# major rules to create files
#
all: 	app jar 

jar:	app 
	@echo Creating binary archive ...
	@if [ ! -d jar ]; then mkdir jar; fi
	@$(JAR) cvfm jar/$(PKGNAME).jar manifest \
	  `find $(SRCDIR) -name \*.class -follow` \
	  `find $(SRCDIR) -name \*.conf` > /dev/null
	@echo jar/$(PKGNAME).jar

# 
# application dependencies
#
app:
	@find $(SRCDIR) -name \*.java | sed 's/java$$/class/' | xargs make
	@echo Done.

clean:
	-find . -name "*.class" -exec rm -f {} \; -print

realclean: clean
	-rm -f jar/$(PKGNAME).jar jar/$(PKGNAME)-src.jar

include ../../../make-include.mk

install:
	cp jar/WebTelnet.jar  $(TOP_DIR)/www/htdocs

un-install:
	cd $(TOP_DIR)/www/htdocs; rm -f WebTelnet.jar

