config = {
	container: "#AST"
};

ROOT00665408 = {
	text: { name: "ROOT" }
};

DECL0066E710 = {
	parent: ROOT00665408,
	text: { name: "INTEGER" }
};

ID00665548 = {
	parent: DECL0066E710,
	text: { name: "a" }
};

DECL0066E568 = {
	parent: ROOT00665408,
	text: { name: "INTEGER" }
};

ID0066E4F8 = {
	parent: DECL0066E568,
	text: { name: "b" }
};

ASSIGN006714D8 = {
	parent: ROOT00665408,
	text: { name: "=" }
};

ID00671468 = {
	parent: ASSIGN006714D8,
	text: { name: "a" }
};

INT0066A240 = {
	parent: ASSIGN006714D8,
	text: { name: "4" }
};

ASSIGN006715A0 = {
	parent: ROOT00665408,
	text: { name: "=" }
};

ID00671530 = {
	parent: ASSIGN006715A0,
	text: { name: "b" }
};

INT00669FC0 = {
	parent: ASSIGN006715A0,
	text: { name: "5" }
};

IF006723D0 = {
	parent: ROOT00665408,
	text: { name: "IF" }
};

COND00671788 = {
	parent: IF006723D0,
	text: { name: "<" }
};

BINOP006716C0 = {
	parent: COND00671788,
	text: { name: "-" }
};

BINOP00671668 = {
	parent: BINOP006716C0,
	text: { name: "*" }
};

ID006715F8 = {
	parent: BINOP00671668,
	text: { name: "a" }
};

INT0066A0B0 = {
	parent: BINOP00671668,
	text: { name: "6" }
};

ID00671718 = {
	parent: BINOP006716C0,
	text: { name: "b" }
};

BINOP00671850 = {
	parent: COND00671788,
	text: { name: "+" }
};

ID006717E0 = {
	parent: BINOP00671850,
	text: { name: "b" }
};

BINOP00672270 = {
	parent: BINOP00671850,
	text: { name: "*" }
};

INT0066A100 = {
	parent: BINOP00672270,
	text: { name: "3" }
};

BINOP00672588 = {
	parent: BINOP00672270,
	text: { name: "/" }
};

INT0066A4C0 = {
	parent: BINOP00672588,
	text: { name: "10" }
};

ID006718A8 = {
	parent: BINOP00672588,
	text: { name: "a" }
};

ASSIGN006724D8 = {
	parent: IF006723D0,
	text: { name: "=" }
};

ID00671978 = {
	parent: ASSIGN006724D8,
	text: { name: "b" }
};

INT00669F20 = {
	parent: ASSIGN006724D8,
	text: { name: "6" }
};

ASSIGN00672320 = {
	parent: IF006723D0,
	text: { name: "=" }
};

ID00672A50 = {
	parent: ASSIGN00672320,
	text: { name: "a" }
};

INT0066A650 = {
	parent: ASSIGN00672320,
	text: { name: "3" }
};

WHILE00672428 = {
	parent: IF006723D0,
	text: { name: "WHILE" }
};

COND00672950 = {
	parent: WHILE00672428,
	text: { name: ">" }
};

ID00672AC0 = {
	parent: COND00672950,
	text: { name: "b" }
};

ID00672B30 = {
	parent: COND00672950,
	text: { name: "a" }
};

ASSIGN006728F8 = {
	parent: WHILE00672428,
	text: { name: "=" }
};

ID00674B00 = {
	parent: ASSIGN006728F8,
	text: { name: "a" }
};

BINOP006729A8 = {
	parent: ASSIGN006728F8,
	text: { name: "+" }
};

ID006749B0 = {
	parent: BINOP006729A8,
	text: { name: "a" }
};

INT0066A510 = {
	parent: BINOP006729A8,
	text: { name: "1" }
};

DECL00672740 = {
	parent: ROOT00665408,
	text: { name: "INTEGER" }
};

ID006748D0 = {
	parent: DECL00672740,
	text: { name: "c" }
};

ASSIGN006726E8 = {
	parent: ROOT00665408,
	text: { name: "=" }
};

ID00674B70 = {
	parent: ASSIGN006726E8,
	text: { name: "c" }
};

BINOP006722C8 = {
	parent: ASSIGN006726E8,
	text: { name: "+" }
};

ID00674A20 = {
	parent: BINOP006722C8,
	text: { name: "a" }
};

ID00674F60 = {
	parent: BINOP006722C8,
	text: { name: "b" }
};

UNARY00672378 = {
	parent: ROOT00665408,
	text: { name: "RET" }
};

ID00674A90 = {
	parent: UNARY00672378,
	text: { name: "c" }
};

simple_chart_config = [
    config, ROOT00665408, DECL0066E710, ID00665548, DECL0066E568, ID0066E4F8, ASSIGN006714D8, ID00671468, INT0066A240, ASSIGN006715A0, ID00671530, INT00669FC0, IF006723D0, COND00671788, BINOP006716C0, BINOP00671668, ID006715F8, INT0066A0B0, ID00671718, BINOP00671850, ID006717E0, BINOP00672270, INT0066A100, BINOP00672588, INT0066A4C0, ID006718A8, ASSIGN006724D8, ID00671978, INT00669F20, ASSIGN00672320, ID00672A50, INT0066A650, WHILE00672428, COND00672950, ID00672AC0, ID00672B30, ASSIGN006728F8, ID00674B00, BINOP006729A8, ID006749B0, INT0066A510, DECL00672740, ID006748D0, ASSIGN006726E8, ID00674B70, BINOP006722C8, ID00674A20, ID00674F60, UNARY00672378, ID00674A90
];