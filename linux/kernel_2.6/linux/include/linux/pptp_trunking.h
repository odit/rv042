
struct RulePortTable {
	int sport;
	int eport;
	int type;
};

struct PortTableHead {
	int TableSize;
	struct RulePortTable *RulePortPtr;
};

struct PtrunkingRuleData {
	struct PtrunkingRuleData *next;
	char sip[20];
	char eip[20];
	char prio[10];
};
