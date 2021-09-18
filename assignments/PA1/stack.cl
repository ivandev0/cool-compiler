class Stack inherits IO {
	top: String;
	tail: Stack;

	(* protected *) init(value: String, newTail: Stack): Stack {{
		top <- value;
		tail <- newTail;
		self;
	}};

	push(value: String): Stack { (new Stack).init(value, self) };

	pop(): Stack { tail };

	top(): String { top };

	isEmpty(): Bool { false };

	print_stack() : Object {{
        out_string(top).out_string("\n");
        tail.print_stack();
    }};
};

class EmptyStack inherits Stack {
	push(value: String): Stack { (new Stack).init(value, self) };

	pop(): Stack {{ abort(); self; }};

	top(): String {{ abort(); ""; }};

	isEmpty(): Bool { true };

	print_stack() : Object { out_string("") };
};

class StackCommandDispatcher {
	evalStackCommand: EvalStackCommand <- new EvalStackCommand;
	printStackCommand: PrintStackCommand <- new PrintStackCommand;
	switchStackCommand: SwitchStackCommand <- new SwitchStackCommand;
	sumStackCommand: SumStackCommand <- new SumStackCommand;
	intStackCommand: IntStackCommand <- new IntStackCommand;

	dispatch(stack: Stack, command: String): Stack {{
		if evalStackCommand.isApplicable(command) then 
			if stack.isEmpty() then stack 
			else let top: String <- stack.top() in { 
				if switchStackCommand.isApplicable(top) then switchStackCommand.apply(stack.pop(), top) 
				else if sumStackCommand.isApplicable(top) then sumStackCommand.apply(stack.pop(), top) 	
				else stack fi fi; 
			} fi
		else if printStackCommand.isApplicable(command) then printStackCommand.apply(stack, command) 
		else intStackCommand.apply(stack, command) fi fi;
	}};
};


class StackCommand {
	isApplicable(command: String): Bool {{ abort(); false; }};
	apply(stack: Stack, command: String): Stack {{ abort(); stack; }};
};

class EvalStackCommand inherits StackCommand {
	switchStackCommand: SwitchStackCommand <- new SwitchStackCommand;
	sumStackCommand: SumStackCommand <- new SumStackCommand;

	isApplicable(command: String): Bool { command = "e" };
	apply(stack: Stack, command: String): Stack {{ abort(); stack; }};
};

class PrintStackCommand inherits StackCommand {
	isApplicable(command: String): Bool { command = "d" };
	apply(stack: Stack, command: String): Stack {{ stack.print_stack(); stack; }};
};

class SwitchStackCommand inherits StackCommand {
	isApplicable(command: String): Bool { command = "s" };
	apply(stack: Stack, command: String): Stack { 
		let s1 : String <- stack.top() in {
			stack <- stack.pop();
			let s2 : String <- stack.top() in {
				stack.pop().push(s1).push(s2);
			};
		}
	};
};

class SumStackCommand inherits StackCommand {
	a2i: A2I <- new A2I;

	isApplicable(command: String): Bool { command = "+" };
	apply(stack: Stack, command: String): Stack { 
		let s1 : String <- stack.top() in {
			stack <- stack.pop();
			let s2 : String <- stack.top() in {
				stack.pop().push(a2i.i2a(a2i.a2i(s1) + a2i.a2i(s2)));
			};
		}
	};
};

class IntStackCommand inherits StackCommand {
	isApplicable(command: String): Bool { false };
	apply(stack: Stack, command: String): Stack { stack.push(command) };
};

class Main inherits IO {
	dispatcher: StackCommandDispatcher <- new StackCommandDispatcher;
	stack: Stack <- new EmptyStack;

	main() : Object {
		let command: String <- in_string() in {
			while not (command = "x") loop { 
		    	stack <- dispatcher.dispatch(stack, command);
				command <- in_string();
	    	} pool;
		}		
	};
};
