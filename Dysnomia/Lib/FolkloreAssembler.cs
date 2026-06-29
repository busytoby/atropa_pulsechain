using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using Dysnomia.Domain;

namespace Dysnomia.Lib
{
    public static class FolkloreAssembler
    {
        private static readonly Dictionary<string, byte> RegisterMap = new Dictionary<string, byte>(StringComparer.OrdinalIgnoreCase)
        {
            { "Base", 0 },
            { "Secret", 1 },
            { "Signal", 2 },
            { "Contour", 3 },
            { "Pole", 4 },
            { "Coordinate", 5 },
            { "Foundation", 6 },
            { "Element", 7 },
            { "Dynamo", 8 },
            { "Monopole", 9 },
            { "Barn", 10 },
            { "Chin", 11 },
            { "Identity", 12 }
        };

        // Parses a register string name and returns its byte index
        private static byte ParseRegister(string regStr)
        {
            string clean = regStr.Trim().Replace(",", "");
            if (RegisterMap.TryGetValue(clean, out byte index))
            {
                return index;
            }
            throw new ArgumentException($"Invalid Auncient register name: '{clean}'");
        }

        // Assembles a single line of Folklore assembly into a 32-byte big-endian Yul word
        public static byte[] AssembleInstruction(string line)
        {
            // Remove comments and trim
            int commentIdx = line.IndexOf(';');
            if (commentIdx >= 0) line = line.Substring(0, commentIdx);
            line = line.Trim();

            if (string.IsNullOrEmpty(line))
            {
                return Array.Empty<byte>();
            }

            // Split into tokens
            string[] tokens = Regex.Split(line, @"\s+");
            if (tokens.Length == 0) return Array.Empty<byte>();

            string opcode = tokens[0].ToUpper();
            byte op = 0;
            byte rDst = 0;
            byte rSrc1 = 0;
            byte val = 0;

            switch (opcode)
            {
                // LOAD <DstRegister> <ImmediateValue>
                case "LOAD":
                    if (tokens.Length < 3) throw new FormatException("LOAD requires DstRegister and ImmediateValue");
                    op = 1;
                    rDst = ParseRegister(tokens[1]);
                    val = Convert.ToByte(tokens[2].Trim().Replace(",", ""));
                    break;

                // ADD <DstRegister> <SrcRegister1> <SrcRegister2>
                case "ADD":
                    if (tokens.Length < 4) throw new FormatException("ADD requires DstRegister, SrcRegister1, and SrcRegister2");
                    op = 2;
                    rDst = ParseRegister(tokens[1]);
                    rSrc1 = ParseRegister(tokens[2]);
                    val = ParseRegister(tokens[3]); // Target value/src2 is loaded into val slot for ADD
                    break;

                // JMPZ <ConditionRegister> <TargetOffset>
                case "JMPZ":
                    if (tokens.Length < 3) throw new FormatException("JMPZ requires ConditionRegister and TargetOffset");
                    op = 4;
                    rSrc1 = ParseRegister(tokens[1]);
                    val = Convert.ToByte(tokens[2].Trim().Replace(",", ""));
                    break;

                default:
                    throw new FormatException($"Unknown opcode mnemonic: '{opcode}'");
            }

            // Build 32-byte big-endian EVM word
            byte[] instruction = new byte[32];
            instruction[0] = op;
            instruction[1] = rDst;
            instruction[2] = rSrc1;
            instruction[3] = val;

            return instruction;
        }
    }
}
