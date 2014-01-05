JAVAC = javac -classpath .:$$CLASSPATH
JAR = jar
INSTALL_DIR = $(TOP_DIR)/www/htdocs
SRCS = $(shell find . -name '*.java' -print 2>/dev/null)

all: vncviewer.jar 

vncviewer.jar: $(SRCS)
	$(RM) `find . -name '*.class' -print`
	$(JAVAC) -d . $^
	$(JAR) cmf MANIFEST.MF $@ `find . -name '*.class' -print`

clean:
	$(RM) `find . -name '*.class' -print`
	$(RM) vncviewer.jar

include ../../../make-include.mk
install:
	if [ -d $(INSTALL_DIR) ]; then \
	  cp -pf vncviewer.jar $(INSTALL_DIR); \
	fi

un-install:
