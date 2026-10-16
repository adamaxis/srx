import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;

public class FindShadowrunReferences extends GhidraScript {
    @Override
    public void run() throws Exception {
        long[] targets = {0x5b7d8L, 0x1b6498L, 0x1b6527L,
                          0x1167ebL, 0x11681cL,
                          0xffde1aL, 0xffde1eL, 0xffde22L,
                          0xffe114L, 0xffde14L,
                          0xfffbf9L, 0xffe162L,
                          0xffddc6L, 0xffddc4L,
                          0x2b34L, 0x2bd2L, 0x2fa4L, 0x2f32L};
        for (long target : targets) {
            Address address = toAddr(target);
            println(String.format("TARGET %06x", target));
            var references = getReferencesTo(address);
            for (var reference : references) {
                Instruction instruction = getInstructionAt(reference.getFromAddress());
                if (instruction != null) {
                    println(instruction.getAddressString(false, true) + " " +
                        instruction.toString());
                } else {
                    println(reference.toString());
                }
            }
        }
    }
}
