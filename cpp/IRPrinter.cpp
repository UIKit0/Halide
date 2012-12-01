#include "IRPrinter.h"
#include "IROperator.h"
#include "IR.h"

#include <iostream>

namespace HalideInternal {

    using std::ostream;
    using std::endl;

    void IRPrinter::test() {
        Type i32 = Int(32);
        Type f32 = Float(32);
        Expr x = new Var(i32, "x");
        Expr y = new Var(i32, "y");
        std::cout << (x + 3) * (y / 2 + 17) << endl;

        Stmt store = new Store("buf", (x * 17) / (x - 3), y - 1);
        Stmt for_loop = new For("x", -2, y + 2, For::Parallel, store);
        vector<Expr> args(1); args[0] = x % 3;
        Expr call = new Call(i32, "buf", args, Call::Halide);
        Stmt store2 = new Store("out", call + 1, x);
        Stmt for_loop2 = new For("x", 0, y, For::Vectorized , store2);
        Stmt pipeline = new Pipeline("buf", for_loop, Stmt(), for_loop2);
        Stmt assertion = new AssertStmt(y > 3, "y is greater than 3");
        Stmt block = new Block(assertion, pipeline);
        Stmt let_stmt = new LetStmt("y", 17, block);
        Stmt allocate = new Allocate("buf", f32, 1023, let_stmt);

        std::cout << allocate << endl;
    }

    ostream &operator<<(ostream &out, Type type) {
        switch (type.t) {
        case Type::Int:
            out << 'i';
            break;
        case Type::UInt:
            out << 'u';
            break;
        case Type::Float:
            out << 'f';
            break;
        default:
            assert(false && "Malformed type");
        }
        out << type.bits;
        if (type.width > 1) out << 'x' << type.width;
        return out;
    }

    ostream &operator<<(ostream &out, For::ForType type) {
        switch (type) {
        case For::Serial:
            out << "for";
            break;
        case For::Parallel:
            out << "parallel";
            break;
        case For::Unrolled:
            out << "unrolled";
            break;
        case For::Vectorized:
            out << "vectorized";
            break;
        default:
            assert(false && "Malformed for type");
        }
        return out;
    }

    ostream &operator<<(ostream &stream, Expr ir) {
        IRPrinter p(stream);
        p.print(ir);
        return stream;
    }

    ostream &operator<<(ostream &stream, Stmt ir) {
        IRPrinter p(stream);
        p.print(ir);
        return stream;
    }

    IRPrinter::IRPrinter(ostream &s) : stream(s), indent(0) {}

    void IRPrinter::print(Expr ir) {
        ir.accept(this);
    }

    void IRPrinter::print(Stmt ir) {
        ir.accept(this);
    }


    void IRPrinter::do_indent() {
        for (int i = 0; i < indent; i++) stream << ' ';
    }
    
    void IRPrinter::visit(const IntImm *op) {
        stream << op->value;
    }
    
    void IRPrinter::visit(const FloatImm *op) {
        stream << op->value;
    }
    
    void IRPrinter::visit(const Cast *op) { 
        stream << op->type << '(';
        print(op->value);
        stream << ')';
    }
    
    void IRPrinter::visit(const Var *op) {
        // omit the type
        stream << op->name;
    }
    
    void IRPrinter::visit(const Add *op) {
        stream << '(';
        print(op->a);
        stream << " + ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const Sub *op) {
        stream << '(';
        print(op->a);
        stream << " - ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const Mul *op) {
        stream << '(';
        print(op->a);
        stream << "*";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const Div *op) {
        stream << '(';
        print(op->a);
        stream << "/";
        print(op->b);
        stream << ')';
    }
            
    void IRPrinter::visit(const Mod *op) {
        stream << '(';
        print(op->a);
        stream << " % ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const Min *op) {
        stream << "min(";
        print(op->a);
        stream << ", ";
        print(op->b);
        stream << ")";
    }

    void IRPrinter::visit(const Max *op) {
        stream << "max(";
        print(op->a);
        stream << ", ";
        print(op->b);
        stream << ")";
    }

    void IRPrinter::visit(const EQ *op) {
        stream << '(';
        print(op->a);
        stream << " == ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const NE *op) {
        stream << '(';
        print(op->a);
        stream << " != ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const LT *op) {
        stream << '(';
        print(op->a);
        stream << " < ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const LE *op) {
        stream << '(';
        print(op->a);
        stream << " <= ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const GT *op) {
        stream << '(';
        print(op->a);
        stream << " > ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const GE *op) {
        stream << '(';
        print(op->a);
        stream << " >= ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const And *op) {
        stream << '(';
        print(op->a);
        stream << " && ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const Or *op) {
        stream << '(';
        print(op->a);
        stream << " || ";
        print(op->b);
        stream << ')';
    }

    void IRPrinter::visit(const Not *op) {
        stream << '!';
        print(op->a);
    }

    void IRPrinter::visit(const Select *op) {
        stream << "select(";
        print(op->condition);
        stream << ", ";
        print(op->true_value);
        stream << ", ";
        print(op->false_value);
        stream << ")";
    }

    void IRPrinter::visit(const Load *op) {
        stream << op->buffer << "[";
        print(op->index);
        stream << "]";
    }

    void IRPrinter::visit(const Ramp *op) {
        stream << "ramp(";
        print(op->base);
        stream << ", ";
        print(op->stride);
        stream << ", " << op->width << ")";
    }

    void IRPrinter::visit(const Broadcast *op) {
        stream << "broadcast(";
        print(op->value);
        stream << ", " << op->width << ")";
    }

    void IRPrinter::visit(const Call *op) {
        stream << op->buffer << "(";
        for (size_t i = 0; i < op->args.size(); i++) {
            print(op->args[i]);
            if (i < op->args.size() - 1) {
                stream << ", ";
            }
        }
        stream << ")";
    }

    void IRPrinter::visit(const Let *op) {
        stream << "(let " << op->name << " = ";
        print(op->value);
        stream << " in ";
        print(op->body);
        stream << ")";
    }

    void IRPrinter::visit(const LetStmt *op) {
        do_indent();
        stream << "let " << op->name << " = ";
        print(op->value);
        stream << endl;

        do_indent();
        print(op->body);
    }

    void IRPrinter::visit(const PrintStmt *op) {
        do_indent();
        stream << "print(" << op->prefix;
        for (size_t i = 0; i < op->args.size(); i++) {
            stream << ", ";
            print(op->args[i]);
        }
        stream << ")" << endl;
    }

    void IRPrinter::visit(const AssertStmt *op) {
        do_indent();
        stream << "assert(";
        print(op->condition);
        stream << ", \"" << op->message << "\")" << endl;
    }

    void IRPrinter::visit(const Pipeline *op) {


        do_indent();
        stream << "produce " << op->buffer << " {" << endl;
        indent += 2;
        print(op->produce);
        indent -= 2;

        if (op->update.defined()) {
            do_indent();
            stream << "} update {" << endl;
            indent += 2;
            print(op->update);
            indent -= 2;
        }
        
        do_indent();
        stream << "} consume {" << endl;
        indent += 2;
        print(op->consume);
        indent -= 2;

        do_indent();
        stream << "}" << endl;
    }

    void IRPrinter::visit(const For *op) {

        do_indent();
        stream << op->for_type << " (" << op->name << ", ";
        print(op->min);
        stream << ", ";
        print(op->extent);
        stream << ") {" << endl;
        
        indent += 2;
        print(op->body);
        indent -= 2;

        do_indent();
        stream << "}" << endl;
    }

    void IRPrinter::visit(const Store *op) {
        do_indent();
        stream << op->buffer << "[";
        print(op->index);
        stream << "] = ";
        print(op->value);
        stream << endl;
    }

    void IRPrinter::visit(const Provide *op) {
        do_indent();
        stream << op->buffer << "(";
        for (size_t i = 0; i < op->args.size(); i++) {
            print(op->args[i]);
            if (i < op->args.size() - 1) stream << ", ";
        }
        stream << ") = ";
        print(op->value);
        stream << endl;
    }

    void IRPrinter::visit(const Allocate *op) {
        do_indent();
        stream << "allocate " << op->buffer << "[" << op->type << " * ";
        print(op->size);
        stream << "]" << endl;
        print(op->body);

        do_indent();
        stream << "free " << op->buffer << endl;
    }

    void IRPrinter::visit(const Realize *op) {
        do_indent();
        stream << "realize " << op->buffer << "(";
        for (size_t i = 0; i < op->bounds.size(); i++) {
            stream << "[";
            print(op->bounds[i].first);
            stream << ", ";
            print(op->bounds[i].second);
            stream << "]";
            if (i < op->bounds.size() - 1) stream << ", ";
        }
        stream << ") {" << endl;

        indent += 2;
        print(op->body);
        indent -= 2;

        do_indent();
        stream << "}" << endl;
    }

    void IRPrinter::visit(const Block *op) {
        print(op->first);
        if (op->rest.defined()) print(op->rest);
    }

    
}