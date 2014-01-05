include ../../../../../make-include.mk
BIN=adminBookmarkDelete editglobal editgroups editusers policyDelete userDelete groupDelete users addbookmarkadmin bookmarkadmin policyadmin addpolicy adduser addgroup policyrange policyall policyresource editPolicy showPolicy userattributes 

HTML=users?.html editglobal?.html editgroups?.html editusers?.html addbookmarkadmin.html adduser?.html addgroup?.html policyadmin?.html policyrange?.html policyall?.html policyresource?.html popbottom.html showPolicy?.html userattributes?.html 
ifdef CONFIG_IPV6
BIN+=policy6admin policy6range policy6all
HTML+=policy6admin?.html policy6range?.html policy6all?.html
endif

ifeq "$(CONFIG_EPS)" "y"
BIN+=epsglobalpolicy epsgrouppolicy
HTML+=epsglobalpolicy?.html epsgrouppolicy?.html
endif 

OTHERS=

all: $(BIN)
include ../../../cgi-include.mk

policyrange: policyrange.c
	${CC} $^  $(CFLAGS) -o $@

policyall: policyall.c
	${CC} $^  $(CFLAGS) -o $@

policyresource: policyresource.c
	${CC} $^  $(CFLAGS) -o $@

addgroup: addgroup.c
	${CC} $^  $(CFLAGS) -o $@

adduser: adduser.c
	${CC} $^  $(CFLAGS) -o $@

policyadmin: policyadmin.c
	${CC} $^  $(CFLAGS) -o $@

ifdef CONFIG_IPV6
policy6admin: policy6admin.c
	${CC} $^  $(CFLAGS) -o $@

policy6range: policy6range.c
	${CC} $^  $(CFLAGS) -o $@

policy6all: policy6all.c
	${CC} $^  $(CFLAGS) -o $@

endif

bookmarkadmin: bookmarkadmin.c
	${CC} $^  $(CFLAGS) -o $@

addpolicy: addpolicy.c
	${CC} $^  $(CFLAGS) -o $@

adminBookmarkDelete: adminBookmarkDelete.c
	${CC} $^  $(CFLAGS) -o $@

addbookmarkadmin: addbookmarkadmin.c
	${CC} $^  $(CFLAGS) -o $@

users: users.c
	${CC} $^  $(CFLAGS) -o $@

editglobal: editglobal.c
	${CC} $^  $(CFLAGS) -o $@

editgroups: editgroups.c
	${CC} $^  $(CFLAGS) -o $@

editusers: editusers.c
	${CC} $^  $(CFLAGS) -o $@

policyDelete: policyDelete.c
	${CC} $^  $(CFLAGS) -o $@

userDelete: userDelete.c
	${CC} $^  $(CFLAGS) -o $@

groupDelete: groupDelete.c
	${CC} $^  $(CFLAGS) -o $@

editPolicy: editPolicy.c
	${CC} $^  $(CFLAGS) -o $@

showPolicy: showPolicy.c
	${CC} $^  $(CFLAGS) -o $@

userattributes: userattributes.c
	${CC} $^  $(CFLAGS) -o $@

ifdef CONFIG_EPS
epsglobalpolicy: epsglobalpolicy.c
	${CC} $^  $(CFLAGS) -o $@

epsgrouppolicy: epsgrouppolicy.c
	${CC} $^  $(CFLAGS) -o $@
endif
