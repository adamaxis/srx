import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.Instruction;

public class DumpShadowrunListing extends GhidraScript {
    @Override
    public void run() throws Exception {
        long[][] ranges = {
            {0x0700L, 0x0c00L}, {0x2800L, 0x2910L},
            {0x0f5cL, 0x0fcaL}, {0x131cL, 0x14a8L},
            {0x2a00L, 0x3020L}, {0x2b34L, 0x2c20L},
            {0x3074L, 0x30b0L}, {0x42b0L, 0x4390L},
            {0x16600L, 0x166b0L}, {0x21950L, 0x219c0L},
            {0x1dfe0L, 0x1e080L},
            {0x566d8L, 0x56750L},
            {0x10d30L, 0x10f4eL}, {0x10f50L, 0x11134L},
            {0x11136L, 0x1118eL}, {0x11190L, 0x1118eL},
            {0x11442L, 0x11590L}, {0x11592L, 0x11622L},
            {0x11624L, 0x1170aL}, {0x1170cL, 0x1173eL},
            {0x11740L, 0x118c4L}, {0x118c6L, 0x118feL},
            {0x11928L, 0x11b56L}, {0x12148L, 0x12290L},
            {0x3602L, 0x36a0L}, {0x3d80L, 0x3e80L},
            {0xaff2L, 0xb080L}, {0x5fdcL, 0x6050L},
            {0x600eL, 0x60d0L},
            {0x0e9cL, 0x0f20L}, {0x1200L, 0x1260L},
            {0x12c36L, 0x12c5aL},
            {0x12a54L, 0x12c20L}, {0x12be0L, 0x12c80L},
            {0x12c74L, 0x12e9cL},
            {0x12e9eL, 0x13300L},
            {0x0ffeL, 0x1080L}, {0x10918L, 0x10990L},
            {0x111d0L, 0x11442L}, {0x11442L, 0x114c0L}
            , {0x0c0daL, 0x0c1a0L}
            , {0x01530L, 0x01610L}
            , {0x5c8c6L, 0x5c9d0L}
            , {0x01728L, 0x01872L}
            , {0x01A9EL, 0x01C20L}
            , {0x01c1eL, 0x01c6aL}
            , {0x01CFCL, 0x01D00L}
            , {0x01c6aL, 0x01f00L}
            , {0x0c000L, 0x0c0daL}
            , {0x012f6L, 0x0131cL}
            , {0x011c0L, 0x01200L}
            , {0x010d30L, 0x010f50L}
            , {0x0c216L, 0x0c2c0L}
            , {0x59092L, 0x59720L}
            , {0x167aeL, 0x16880L}
            , {0x16884L, 0x16a26L}
            , {0x16a26L, 0x16b20L}
            , {0x16afaL, 0x16be0L}
            , {0x16b7cL, 0x16c40L}
            , {0x16e30L, 0x16eb0L}
            , {0x0b7b8L, 0x0b8a0L}
            , {0x172cdL, 0x17380L}
            , {0x01cf2L, 0x01e20L}
            , {0x0c376L, 0x0c3e6L}
            , {0x0c3e6L, 0x0c4a0L}
            , {0x10b5aL, 0x10bd0L}
            , {0x10f50L, 0x110bcL}
            , {0x0960L, 0x0b40L}
            , {0x0a00L, 0x0a80L}
            , {0x0b40L, 0x0c00L}
            , {0x01406L, 0x014c0L}
            , {0x01368L, 0x01406L}
            , {0x0c0c2L, 0x0c0daL}
            , {0x0c216L, 0x0c276L}
            , {0x55e62L, 0x56020L}
            , {0x1d6900L, 0x1d6b20L}
            , {0x1dba00L, 0x1dbc20L}
            , {0x12c36L, 0x12d20L}
            , {0x1dba0L, 0x1dc20L}
            , {0x104f0L, 0x10560L}
            , {0x0b900L, 0x0bd00L}
            , {0x0b400L, 0x0b7b8L}
            , {0x164a8L, 0x1670dL}
            , {0x136deL, 0x13730L}
            , {0x1db80L, 0x1dbf0L}
            , {0x0f400L, 0x0f600L}
            , {0x1ffb0L, 0x20500L}
            , {0x53a24L, 0x53b20L}
        };
        java.io.PrintWriter file = new java.io.PrintWriter(getScriptArgs()[0]);
        for (long[] range : ranges) {
            println("===== " + Long.toHexString(range[0]) + " =====");
            file.println("===== " + Long.toHexString(range[0]) + " =====");
            long offset = range[0];
            while (offset <= range[1]) {
                Instruction instruction = getInstructionAt(toAddr(offset));
                if (instruction == null) {
                    disassemble(toAddr(offset));
                    instruction = getInstructionAt(toAddr(offset));
                }
                if (instruction == null) {
                    offset += 2;
                    continue;
                }
                String line = instruction.getAddressString(false, true) + "  " +
                    instruction.getMnemonicString();
                for (int operand = 0; operand < instruction.getNumOperands(); operand++) {
                    String operands = instruction.getDefaultOperandRepresentation(operand);
                    if (operand == 0) line += " "; else line += ", ";
                    line += operands;
                }
                println(line);
                file.println(line);
                offset = instruction.getAddress().getOffset() + instruction.getLength();
            }
        }
        file.close();
    }
}
