import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Instruction;

public class DumpShadowrunActionListing extends GhidraScript {
    @Override
    public void run() throws Exception {
        long[][] ranges = {
            {0x12290L, 0x12a53L},
            {0x12600L, 0x12a53L},
            {0x10f20L, 0x11090L},
            {0x12d00L, 0x12f20L}
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
                    line += operand == 0 ? " " : ", ";
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
