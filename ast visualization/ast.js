config = {
	container: "#AST"
};

ROOT00975388 = {
	text: { name: "ROOT" }
};

ASSIGN00975660 = {
	parent: ROOT00975388,
	text: { name: "=" }
};

ID009753C8 = {
	parent: ASSIGN00975660,
	text: { name: "a" }
};

INT0097EF18 = {
	parent: ASSIGN00975660,
	text: { name: "4" }
};

ASSIGN00975518 = {
	parent: ROOT00975388,
	text: { name: "=" }
};

ID009754C8 = {
	parent: ASSIGN00975518,
	text: { name: "b" }
};

INT0097EF88 = {
	parent: ASSIGN00975518,
	text: { name: "3" }
};

IF00981830 = {
	parent: ROOT00975388,
	text: { name: "IF" }
};

COND00981C50 = {
	parent: IF00981830,
	text: { name: "<" }
};

BINOP0097A2B0 = {
	parent: COND00981C50,
	text: { name: "-" }
};

BINOP0097A270 = {
	parent: BINOP0097A2B0,
	text: { name: "*" }
};

ID009756A0 = {
	parent: BINOP0097A270,
	text: { name: "a" }
};

INT0097E930 = {
	parent: BINOP0097A270,
	text: { name: "6" }
};

ID0097A2F0 = {
	parent: BINOP0097A2B0,
	text: { name: "b" }
};

BINOP00983860 = {
	parent: COND00981C50,
	text: { name: "+" }
};

ID00983508 = {
	parent: BINOP00983860,
	text: { name: "b" }
};

BINOP00983F60 = {
	parent: BINOP00983860,
	text: { name: "*" }
};

INT0097ECE8 = {
	parent: BINOP00983F60,
	text: { name: "3" }
};

BINOP00983DA0 = {
	parent: BINOP00983F60,
	text: { name: "/" }
};

INT0097EA10 = {
	parent: BINOP00983DA0,
	text: { name: "10" }
};

ID009830F8 = {
	parent: BINOP00983DA0,
	text: { name: "a" }
};

ASSIGN009838A0 = {
	parent: IF00981830,
	text: { name: "=" }
};

ID00983468 = {
	parent: ASSIGN009838A0,
	text: { name: "b" }
};

INT0097EAF0 = {
	parent: ASSIGN009838A0,
	text: { name: "6" }
};

ASSIGN00983A60 = {
	parent: IF00981830,
	text: { name: "=" }
};

ID00983738 = {
	parent: ASSIGN00983A60,
	text: { name: "a" }
};

INT00981B08 = {
	parent: ASSIGN00983A60,
	text: { name: "3" }
};

WHILE00981948 = {
	parent: IF00981830,
	text: { name: "WHILE" }
};

COND00983EA0 = {
	parent: WHILE00981948,
	text: { name: ">" }
};

ID00983788 = {
	parent: COND00983EA0,
	text: { name: "b" }
};

ID009832D8 = {
	parent: COND00983EA0,
	text: { name: "a" }
};

ASSIGN00983DE0 = {
	parent: WHILE00981948,
	text: { name: "=" }
};

ID009833C8 = {
	parent: ASSIGN00983DE0,
	text: { name: "a" }
};

BINOP009838E0 = {
	parent: ASSIGN00983DE0,
	text: { name: "+" }
};

ID00983558 = {
	parent: BINOP009838E0,
	text: { name: "a" }
};

INT00981868 = {
	parent: BINOP009838E0,
	text: { name: "1" }
};

simple_chart_config = [
    config, ROOT00975388, ASSIGN00975660, ID009753C8, INT0097EF18, ASSIGN00975518, ID009754C8, INT0097EF88, IF00981830, COND00981C50, BINOP0097A2B0, BINOP0097A270, ID009756A0, INT0097E930, ID0097A2F0, BINOP00983860, ID00983508, BINOP00983F60, INT0097ECE8, BINOP00983DA0, INT0097EA10, ID009830F8, ASSIGN009838A0, ID00983468, INT0097EAF0, ASSIGN00983A60, ID00983738, INT00981B08, WHILE00981948, COND00983EA0, ID00983788, ID009832D8, ASSIGN00983DE0, ID009833C8, BINOP009838E0, ID00983558, INT00981868
];