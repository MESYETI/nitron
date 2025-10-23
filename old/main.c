#include <stdio.h>

typedef struct DictEntry DictEntry;
typedef struct Inst      Inst;

struct DictEntry {
	bool        used;
	uint8_t     flags;
	DictEntry*  link;
	uint8_t     length;
	char        name[257];
	size_t      codeword;
};

struct Inst {
	void   (*inst)(Inst*);
	size_t param;
};

typedef struct {
	Inst*       insts;
	size_t      instsLen;
	size_t      ip;
	DictEntry** forthCode;
	DictEntry** forthIp;
	DictEntry*  thisWord;

	size_t  dspStack[256];
	size_t* dsp;
	size_t  rspStack[256];
	size_t* rsp;

	DictEntry dictionary[1024];
} VM;

static VM vm;

static void Jump(Inst* inst) {
	-- vm.dsp;
	vm.ip = *vm.dsp;
}

static void PushRsp(Inst* inst) {
	*vm.rsp = inst->param;
	++ vm.rsp;
}

static void PushDsp(Inst* inst) {
	*vm.dsp = inst->param;
	++ vm.dsp;
}

static void PopDsp(Inst* inst) {
	-- vm.dsp;
}

static void Swap(Inst* inst) {
	size_t val = vm.dsp[-1];
	vm.dsp[-1] = vm.dsp[-2];
	vm.dsp[-2] = val;
}

static void Dup(Inst* inst) {
	vm.dsp[0] = vm.dsp[-1];
	++ vm.dsp;
}

static void Over(Inst* inst) {
	vm.dsp[0] = vm.dsp[-2];
	++ vm.dsp;
}

static void Add(Inst* inst) {
	-- vm.dsp;
	vm.dsp[-1] += vm.dsp[0];
}

static void SetIP(Inst* inst) {
	-- vm.dsp;
	vm.forthIp = *vm.dsp;
}

static void GetIP(Inst* inst) {
	*vm.dsp = vm.forthIp;
	++ vm.dsp;
}

static void ToRsp(Inst* inst) {
	-- vm.rsp;
	*vm.dsp = *vm.rsp;
	++ vm.dsp;
}

static void GetCodeWord(Inst* inst) {
	*vm.dsp = vm.thisWord->codeword;
	++ vm.dsp;
}

static void NextCall(Inst* inst) {
	vm.thisWord = *forthIp;
}

/* NEXT macro. */
	// .macro NEXT
	// lodsl
	// jmp *(%eax)
	// .endm

void VM_Init() {
	for (size_t i = 0; i < sizeof(vm.dictionary) / sizeof(DictEntry); ++ i) {
		vm.dictionary[i].used = false;
	}

	size_t idx = 0;
	// docol
	size_t docol = idx;
	vm.insts[idx ++] = (Inst) {&GetIP,       0};
	vm.insts[idx ++] = (Inst) {&ToRsp,       0};
	vm.insts[idx ++] = (Inst) {&GetCodeWord, 0};
	vm.insts[idx ++] = (Inst) {&SetIP,       0};
	vm.insts[idx ++] = (Inst) {&NextCall,    0}; // NEXT
	vm.insts[idx ++] = (Inst) {&GetCodeWord, 0};
	vm.insts[idx ++] = (Inst) {&Jump,        0};
	vm.dictionary[0] = (DictEntry) {
		true, 0, &vm.dictionary[1], 5, "docol", docol
	};
	// drop
	size_t word = idx;
	vm.insts[idx ++] = (Inst) {&PopDsp, 0};
	vm.dictionary[1] = (DictEntry) {
		true, 0, &vm.dictionary[2], 4, "drop", word
	};
	// swap
	word = idx;
	vm.insts[idx ++] = (Inst) {&Swap, 0};
	vm.dictionary[2] = (DictEntry) {
		true, 0, &vm.dictionary[3], 4, "swap", word
	};
	// dup
	word = idx;
	vm.insts[idx ++] = (Inst) {&Dup, 0};
	vm.dictionray[3] = (DictEntry) {
		true, 0, &vm.dictionary[4], 3, "dup", word
	};
	// over
	word = idx;
	vm.insts[idx ++] = (Inst) {&Over, 0};
	vm.dictionary[4] = (DictEntry) {
		true, 0, &vm.dictionary[5], 4, "over", word
	};

	vm.dictionary[0] = (DictEntry) {
		true, 0, NULL, 5, "docol", &DoCol
	};
}
