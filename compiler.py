
# forth grammar
from arpeggio import *
from arpeggio import RegExMatch as _
def fth_comment():		return _(r"\\.*")
def fth_word():			return _(r"(?!if|else|then|do|loop)[^:;()\\\s]+(?=\s|$)")
def fth_call():			return fth_word
def fth_number():		return _(r"[0-9]+(?=\s|$)")
def fth_if():			return Kwd("if"), fth_block, Optional( Kwd("else"), fth_block ), Kwd("then")
def fth_do():			return Kwd("do"), fth_block, Kwd("loop")
def fth_statement():	return [fth_if, fth_do, fth_number, fth_call]
def fth_block():		return ZeroOrMore(fth_statement)
def fth_defcmt():		return _(r"\([^\)]*\)")
def fth_def():			return Kwd(":"), fth_word, fth_defcmt, fth_block, Kwd(";")
def fth_mainblock():	return ZeroOrMore([fth_def, fth_statement])
def fth():				return fth_mainblock, EOF

# opcodes
ops = {
	"nop":		0x00, # ( -- )
	"stop":		0x01, # ( -- )
	"__lit":	0x02, # ( -- n )
	"dup":		0x03, # ( n -- nn )
	"drop":		0x04, # ( n -- )
	"swap":		0x05, # ( ab -- ba )
	">r":		0x06, # ( n - r )
	"r>":		0x07, # ( r - n )
	"__jmp":	0x08, # ( n -- )
	"__cjmp":	0x09, # ( cond n -- )
	"__call":	0x0a, # ( n -- r )
	"__ccall":	0x0b, # ( cond n -- r )
	"__return":	0x0c, # ( r -- )
	"@":		0x0d, # ( addr -- n )
	"!":		0x0e, # ( value addr -- )
	"+":		0x0f, # ( a b -- a+b )
	"-":		0x10, # ( a b -- a-b )
	"*":		0x11, # ( a b -- a*b )
	"/%":		0x12, # ( a b -- a/b a%b )
	"==":		0x13, # ( a b -- a==b )
	"!=":		0x14, # ( a b -- a!=b )
	"<":		0x15, # ( a b -- a<b )
	">":		0x16, # ( a b -- a>b )
	"&":		0x17, # ( a b -- a&b )
	"|":		0x18, # ( a b -- a|b )
	"^":		0x19, # ( a b -- a^b )
	"<<":		0x1a, # ( a b -- a<<b or a>>abs(b) if b is neg )
	"syscall0":	0x1b, # ( n -- )
	"syscall1":	0x1c, # ( arg0 n -- )
	"syscall2":	0x1d, # ( arg0 arg1 n -- )
	"syscall3":	0x1e, # ( arg0 arg1 arg2 n -- )
	"syscall4":	0x1f  # ( arg0 arg1 arg2 arg3 n -- )
}

# ast visitor
# returns list of tuples, each tuple contains two values
# first acts as type, second as plain value
# types are :
# n - number
# o - opcode
# r - reference to a label, will be replaced by number at link
# l - label, used just a placeholder to calculate references later
class FthVisitor(PTNodeVisitor):
	def __init__(self, **kwargs):
		self.words = {}
		self.temp_labels = 0
		super(FthVisitor, self).__init__(defaults = False, **kwargs)

	def visit_fth(self, node, children):
		if len(children) > 1:
			raise ValueError("grammar error")
		out = children[0] + [("o", ops["stop"])] # main app blob
		for k, v in self.words.items():
			out += v # we place main program first so entry point is always at first instruction
		return out

	def visit_fth_def(self, node, children):
		n = children[0]
		if n in self.words:
			raise ValueError("redefinition of %s" % n)
		out = []
		out.append(("l", n))
		out.extend(children[1])
		out.append(("o", ops["__return"]))
		self.words[n] = out
		return None

	def _get_temp_label(self, prefix = ""):
		self.temp_labels += 1
		return "__int_%s%i" % (prefix, self.temp_labels)

	def _flat(self, children):
		out = []
		for a in children:
			if a[0] == "b": # flatten block tuple
				out.extend(a[1])
			else:
				out.append(a)
		return out

	def visit_fth_mainblock(self, node, children): # blocks are needed to flatten return of multiple statements to one op list
		return self._flat(children)
	def visit_fth_block(self, node, children):
		return self._flat(children)

	def visit_fth_statement(self, node, children): # statament might return one or more ops
		if len(children) > 1:
			raise ValueError("grammar error")
		return children[0]

	def visit_fth_if(self, node, children):
		if len(children) == 1: # if children[0] then
			l_end = self._get_temp_label("if")
			out = []
			out.append(("o", ops["__lit"]))		# negate result
			out.append(("n", 0))
			out.append(("o", ops["=="]))
			out.append(("o", ops["__lit"]))
			out.append(("r", l_end))
			out.append(("o", ops["__cjmp"]))	# if true we jump to the end
			out.extend(children[0])				# otherwise we execute children[1] which is ops after else
			out.append(("l", l_end))
			return ("b", out)
		elif len(children) == 2: # if children[0] else children[1] then
			l_else = self._get_temp_label("if")
			l_end = self._get_temp_label("if")
			out = []
			out.append(("o", ops["__lit"]))
			out.append(("r", l_else))
			out.append(("o", ops["__cjmp"]))	# if true we jump to children[0] code
			out.extend(children[1])				# otherwise we execute children[1] which is ops after else
			out.append(("o", ops["__lit"]))
			out.append(("r", l_end))
			out.append(("o", ops["__jmp"]))
			out.append(("l", l_else))
			out.extend(children[0])
			out.append(("l", l_end))
			return ("b", out)
		else:
			raise ValueError("grammar error")

	def visit_fth_do(self, node, children):
		if len(children) != 1:
			raise ValueError("grammar error")

		l_start = self._get_temp_label("do")

		out = []
		out.append(("l", l_start))
		out.append(("o", ops["swap"])) # swap start and end
		out.append(("o", ops[">r"])) # save end pos
		out.append(("o", ops[">r"])) # save start pos
		out.extend(children[0])
		out.append(("o", ops["r>"])) # restore start pos
		out.append(("o", ops["r>"])) # restore end pos
		out.append(("o", ops["swap"])) # swap start and end
		out.append(("o", ops["__lit"])) # inc iterator
		out.append(("n", 1))
		out.append(("o", ops["+"]))
		out.append(("o", ops["dup"])) # 2dup
		out.append(("o", ops[">r"]))
		out.append(("o", ops["swap"]))
		out.append(("o", ops["dup"]))
		out.append(("o", ops[">r"]))
		out.append(("o", ops["swap"]))
		out.append(("o", ops["r>"]))
		out.append(("o", ops["r>"]))
		out.append(("o", ops[">"])) # check if iterator < end value
		out.append(("o", ops["__lit"]))
		out.append(("r", l_start))
		out.append(("o", ops["__cjmp"])) # if true we jump to the start
		out.append(("o", ops["drop"])) # we drop start and end values
		out.append(("o", ops["drop"]))

		return ("b", out)

	def visit_fth_number(self, node, children):
		return ("b", [("o", ops["__lit"]), ("n", int(str(node)))])

	def visit_fth_call(self, node, children):
		if len(children) > 0:
			raise ValueError("grammar error")
		v = str(node)
		if v in ops:
			return ("o", ops[v])
		else:
			return ("b", [("o", ops["__lit"]), ("r", v), ("o", ops["__call"])])

	def visit_fth_word(self, node, children):
		return str(node)

import argparse
parser = argparse.ArgumentParser(description = "NanoForth compiler")
parser.add_argument("-i", "--input", required = True, help = "input file name")
parser.add_argument("-o", "--output", required = True, help = "output file name")
parser.add_argument("-s", "--disasm", dest = "disasm", action = "store_true", default = False)
args = vars(parser.parse_args())

# reading file
program_text = ""
with open(args.get("input"), "r") as f:
	program_text = f.read()

# compilation
bytecode = visit_parse_tree(ParserPython(fth, fth_comment, debug = False).parse(program_text), FthVisitor(debug = False))

# label linking
image = []
pos_of = {}
for t, v in bytecode:
	if t == "l":
		pos_of[v] = len(image)
	else:
		image.append((t, v))
for i in range(0, len(image)):
	t = image[i][0]
	v = image[i][1]
	if t == "r":
		if v not in pos_of:
			raise ValueError("not defined label %s" % v)
		image[i] = ("n", pos_of[v])

#from pprint import pprint
#pprint(bytecode)
#pprint(image)

# TODO drop dead code
# TODO word inlining ?

# write image
import struct
with open(args.get("output"), "wb") as f:
	for t, v in image:
		f.write(struct.pack("i", v))

# disasm of the image
if args.get("disasm") == True:
	ops_rev = {}
	for k, v in ops.items():
		ops_rev[v] = k
	pos_of_rev = {}
	for k, v in pos_of.items():
		pos_of_rev[v] = k
	for i, line in enumerate(image):
		t, v = line
		out = "l%4i " % i
		if t == "n":
			out += " n %15i" % v
			if v in pos_of_rev:
				out += " (> %s)" % pos_of_rev.get(v)
		if t == "o":
			out += " o %15s" % ops_rev.get(v, "unknown (%i)" % v)
		if i in pos_of_rev:
			out += " (%s)" % pos_of_rev.get(i)
		print(out)

# TODO
# variables
# arrays
# strings

