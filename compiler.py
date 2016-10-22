
# forth grammar
from arpeggio import *
from arpeggio import RegExMatch as _
def fth_comment():	return _(r"\\.*")
def fth_word():		return _(r"[^:;()\\\s]+(?=\s)")
def fth_number():	return _(r"[0-9]+(?=\s)")
def fth_defcmt():	return _(r"\([^\)]*\)")
def fth_def():		return ":", fth_word, fth_defcmt, ZeroOrMore([fth_number, fth_word]), ';'
def fth():			return ZeroOrMore([fth_def, fth_number, fth_word]), EOF

# ast visitor
class FthVisitor(PTNodeVisitor):
	def __init__(self, **kwargs):
		self.words = dict({})
		super(FthVisitor, self).__init__(defaults = False, **kwargs)
	def visit_fth(self, node, children):
		return self.words, children
	def visit_fth_def(self, node, children):
		if children[0] in self.words:
			raise ValueError("redefinition of %s" % (children[0]))
		self.words[children[0]] = children[1:]
		return None
	def visit_fth_number(self, node, children):
		return ("n", int(str(node)))
	def visit_fth_word(self, node, children):
		return ("w", str(node))

import argparse
parser = argparse.ArgumentParser(description = "NanoForth compiler")
parser.add_argument("-i", "--input", required = True, help = "input file name")
parser.add_argument("-o", "--output", required = True, help = "output file name")
args = vars(parser.parse_args())

# reading file
program_text = ""
with open(args.get("input"), "r") as f:
	program_text = f.read()

# parsing step
words_ast, root_ast = visit_parse_tree(ParserPython(fth, fth_comment, debug = False).parse(program_text), FthVisitor(debug = False))

# opcodes
op_stop    = 0  # ( -- )
op_push    = 1  # ( -- n )
op_drop    = 2  # ( n -- )
op_plus    = 3  # ( nn -- n )
op_minus   = 4  # ( nn -- n )
op_call    = 5  # ( n -- )
op_return  = 6  # ( -- )
op_syscall = 7  # ( nn -- )
builtins = {
	"drop": op_drop,
	"+": op_plus,
	"-": op_minus,
	"syscall": op_syscall,
}

# code generation
def to_bytecode(code, func):
	bytecode = []
	for cmd in code:
		t = cmd[0]
		v = cmd[1]
		if t == "n":
			bytecode.append(("o", op_push))
			bytecode.append(("v", v))
		elif t == "w":
			if v in builtins:
				bytecode.append(("o", builtins[v]))
			else:
				bytecode.append(("o", op_push))
				bytecode.append(("r", v)) # word reference for linking
				bytecode.append(("o", op_call))
	if func:
		bytecode.append(("o", op_return)) # every word ends with return
	else:
		bytecode.append(("o", op_stop)) # main entry point ends with stop
	return bytecode

# getting image together
image = to_bytecode(root_ast, func = False)
pos_of = {}
for word, code in words_ast.items():
	pos_of[word[1]] = len(image)
	image.extend(to_bytecode(code, func = True))

# final linking
for i in range(0, len(image)):
	t = image[i][0]
	v = image[i][1]
	if t == "r":
		if v not in pos_of:
			raise ValueError("not defined word %s" % v)
		image[i] = ("v", pos_of[v])

# write image
import struct
with open(args.get("output"), "wb") as f:
	for t, v in image:
		f.write(struct.pack("i", v))

#from pprint import pprint
#pprint(image)
