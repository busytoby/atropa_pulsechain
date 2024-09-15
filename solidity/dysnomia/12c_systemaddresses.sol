// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./interfaces/12b_chointerface.sol";

contract setaddresses {
    constructor(address ChoAddress) {
        CHOINTERFACE Cho = CHOINTERFACE(ChoAddress);

        Cho.AddSystemAddress("dead", address(0x000000000000000000000000000000000000dEaD));
        Cho.AddSystemAddress("source", address(0x7a20189B297343CF26d8548764b04891f37F3414));
        Cho.AddSystemAddress("trebizond", address(0x903030f7e2d6489F38B0f4F96F9b371ec7960F78));

        Cho.AddSystemAddress("dai", address(0x6B175474E89094C44Da98b954EedeAC495271d0F));
        Cho.AddSystemAddress("usdc", address(0xA0b86991c6218b36c1d19D4a2e9Eb0cE3606eB48));
        Cho.AddSystemAddress("usdt", address(0xdAC17F958D2ee523a2206206994597C13D831ec7));

        Cho.AddSystemAddress("Atropa", address(0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6));
        Cho.AddSystemAddress("EasternLightning", address(0xCD6159D0a1aaE415E0c1504E90A5d741A28afc98));
        Cho.AddSystemAddress("Final", address(0x50E72874dCd7C198d370ac27c7B3cce9f9a0defd));
        Cho.AddSystemAddress("Ka", address(0x83a918056aB9316837Dc48a216119D679D561d91));
        Cho.AddSystemAddress("Buckingham", address(0xe5d3A6e88590fc2A8037D9CCbd816C05B1ff5f11));
        Cho.AddSystemAddress("Wheel", address(0xb9A44De20f26a027e467CB6c2F98766F01904189));
        Cho.AddSystemAddress("Liberty", address(0xFE9b99eCC43cb423408b975cc5ff439e5ABaCb61));
        Cho.AddSystemAddress("Cherokee", address(0xb4C1248812dAbF72cb2e82175b4c0aCffE4D2b10));
        Cho.AddSystemAddress("Dreidel", address(0x8A03b032c5494219B212e5a74A49e2aa7F9d206F));
        Cho.AddSystemAddress("Mantissa", address(0x0EB4EE7d5Ff28cbF68565A174f7E5e186c36B4b3));
        Cho.AddSystemAddress("Neptune", address(0x9A3796Cf41B7CbA6921fd50c3f5204ED6506C3e7));
        Cho.AddSystemAddress("Har", address(0x557F7e30aA6D909Cfe8a229A4CB178ab186EC622));         // ʁ 
        Cho.AddSystemAddress("Theta", address(0xCd19062a6d3019b02A676D72e51D8de7A398dE25));       // Ө
        Cho.AddSystemAddress("CROWS", address(0x203e366A1821570b2f84Ff5ae8B3BdeB48Dc4fa1));
        Cho.AddSystemAddress("Monats", address(0xf8AB3393b1f5CD6184Fb6800A1fC802043C4063e));
        Cho.AddSystemAddress("Legal", address(0x0b1307dc5D90a0B60Be18D2634843343eBc098AF));
        Cho.AddSystemAddress("Scissors", address(0x1b8F9E19360D1dc94295D984b7Ca7eA9b810D9ee));
        Cho.AddSystemAddress("Reading", address(0xf69e9f943674027Cedf05564A8D5A01041d07c62));     // পঁদাে়নুিং
        Cho.AddSystemAddress("Di", address(0x347BC40503E0CE23fE0F5587F232Cd2D07D4Eb89));          // 第作
        Cho.AddSystemAddress("dOWN", address(0x2556F7f8d82EbcdD7b821b0981C38D9dA9439CdD));
        Cho.AddSystemAddress("STAY", address(0x7674516ad438dd67A057fBc1119168d9A7d2a9B1));
        Cho.AddSystemAddress("INDEPENDENCE", address(0x8B090509eAe0fEB4A0B934de1b4345161fA9a62d));
        Cho.AddSystemAddress("LOL", address(0xA63F8061A67ecdbf147Cd1B60f91Cf95464E868D));         // ލ
        Cho.AddSystemAddress("Bullion5", address(0x77Bed67181CeF592472bcb7F97736c560340E006));
        Cho.AddSystemAddress("Bullion8", address(0x2959221675bdF0e59D0cC3dE834a998FA5fFb9F4));    // ⑧
        Cho.AddSystemAddress("TreasuryBill", address(0x463413c579D29c26D59a65312657DFCe30D545A1));
        Cho.AddSystemAddress("TeddyBear", address(0xd6c31bA0754C4383A41c0e9DF042C62b5e918f6d));
        Cho.AddSystemAddress("Poppy", address(0xdE65090088Df0b2d80A5eC6A7B56ECE36ee83ce8));
        Cho.AddSystemAddress("Ojeon", address(0xFa4d9C6E012d946853386113ACbF166deC5465Bb));
        Cho.AddSystemAddress("Yu", address(0x52a4682880E990ebed5309764C7BD29c4aE22deB));          // 유
        Cho.AddSystemAddress("Ying", address(0x271197EFe41073681577CdbBFD6Ee1DA259BAa3c));        // 籯
        Cho.AddSystemAddress("Metis", address(0x36d4Ac3DF7Bf8aa3843Ad40C8b3eB67e3d18b4e1));       // ไมิติซส์
        Cho.AddSystemAddress("Gai", address(0xd6077A029Fb5BEF33b02391D7f0349c345F6DDb1));
        Cho.AddSystemAddress("Disco", address(0xb6936B8e82626405f6E601D54a8292881D86b47D));
        Cho.AddSystemAddress("HOST", address(0x1162104a7b8766784153Dd2D6aC0eCEAecD28117));
        Cho.AddSystemAddress("Dampf", address(0x08Fe5c72173044314A74705089d014a4416Ed71D));
        Cho.AddSystemAddress("DEI", address(0xF77c946C18A77B5DdA5e839dA9818C4D1f087393));
        Cho.AddSystemAddress("Tlingit", address(0x54D88F0c4a738247DadF160923E1b1C5dc4F510f));
        Cho.AddSystemAddress("AbUrbeCondita", address(0x7FB09EE1a2c0E8b6D1c4E19C0248B3CbC0113af6));
        Cho.AddSystemAddress("SIM", address(0xBb341FD5C855c206f5538cc649f90d84Df19b65a));
        Cho.AddSystemAddress("Bin", address(0xf520404CF4fa5B633626333775b05F5dF94E1a9C));
        Cho.AddSystemAddress("PhD", address(0x6236073377AC7e0aB694957dA5d7d4241e72EBc6));
        Cho.AddSystemAddress("Lillies", address(0xE949a217809d1Fab4018E22d6810500399951dAE));
        Cho.AddSystemAddress("Kremlin", address(0x7F51FdB20246D7a673036f11C743E99A4AF01de0));
        Cho.AddSystemAddress("Two", address(0xDf6A16689A893095C721542e5d3CE55bBcc23aC6));         // ㉣
        Cho.AddSystemAddress("Tse", address(0x3d67511733d976800467119264C3d4Cd9FA23041));
        Cho.AddSystemAddress("Frock", address(0x8B8b26bB6C5fD4867339ab2f0acf3aE5129BD2F0));
        Cho.AddSystemAddress("QingDao", address(0xE63191967735C52f5de78CE2471759a9963Ce118));
        Cho.AddSystemAddress("TSFi", address(0x4243568Fa2bbad327ee36e06c16824cAd8B37819));
        Cho.AddSystemAddress("Greenland", address(0xdE4Ef7Ea464c7771803b9838AeA07cE41089b054));
        Cho.AddSystemAddress("Buddhaghosa", address(0x840CBD20A70774BECAc4e932Fff6fb1f5417997F));
        Cho.AddSystemAddress("Zuo", address(0x583d1C1427308f7f96BFd3E0d7A3F9674D8BF8ec));
        Cho.AddSystemAddress("HegelBet", address(0x51C36aA04ffC2139F6d34436d0EDC7f5ffc6D6Fb));
        Cho.AddSystemAddress("Hahnarch", address(0x4a458D04909a42F79d31805762B2abc38ab9407d));
        Cho.AddSystemAddress("Rab", address(0x89E8cD6306AbbAB8e39eeD0D53566d8dC2E02c01));
        Cho.AddSystemAddress("Loan", address(0xeE67825eF27588FAeE39cfefb465eB0A242A740c));
        Cho.AddSystemAddress("Freebies", address(0x48F628c079353ECC4DB75F0d05de9299e083f3C2));

        Cho.AddSystemAddress("WM", address(0xA1BEe1daE9Af77dAC73aA0459eD63b4D93fC6d29));
        Cho.AddSystemAddress("IRC", address(0xD64f26Bcf78df919D587b6743fcFf5b155815bd6));
        Cho.AddSystemAddress("No", address(0x1942Ba1EA7c21a070D70C4eFe64B21694283F23e));
        Cho.AddSystemAddress("Call", address(0xD4FD96BA83d3E6FF1A0Baa44c32Def94e641D97c));        // 𐌎

        Cho.AddSystemAddress("TeddyBear9", address(0x1f737F7994811fE994Fe72957C374e5cD5D5418A));

        Cho.AddSystemAddress("Bond", address(0x25d53961a27791B9D8b2d74FB3e937c8EAEadc38));
        Cho.AddSystemAddress("Bail", address(0x8B16115fF716b4c52706122cb4e974f7a72E5Af1));
        Cho.AddSystemAddress("Writing", address(0x26D5906c4Cdf8C9F09CBd94049f99deaa874fB0b));
        Cho.AddSystemAddress("Sukuk", address(0x72f96a39AC9408b5458E5597BBC22060552dedF4));

        // Cho.AddSystemAddress("FA", address(0xF2be09EB43c1eD2791d0324BaA0649e62CdA4BBF));
        Cho.AddSystemAddress("hhFa", address(0xa28e8aA4d6257157de64a547c90B38C3c540eF72));
        Cho.AddSystemAddress("NoNukes", address(0x174A0ad99c60c20D9B3D94c3095BC1fb9ddEFd62));
        Cho.AddSystemAddress("WenTi", address(0xA537d6F4c1c8F8C41f1004cc34C00e7Db40179Cc));
        Cho.AddSystemAddress("TwoCentClub", address(0x6293373BF0DAE2050641973D49b7b2f71adC91a1));
        Cho.AddSystemAddress("BFF", address(0xE35A842eb1EDca4C710B6c1B1565cE7df13f5996));
        Cho.AddSystemAddress("SECURITY", address(0x2234da59c2D5EDB197594C95dbbA7a99Bcd91230));
        //
        Cho.AddSystemAddress("TRSI", address(0x51A7aaBcCa69B3c0F82b3b9ce5104FDe3efAecE6));
        Cho.AddSystemAddress("BEL", address(0x4C1518286E1b8D5669Fe965EF174B8B4Ae2f017B));
        Cho.AddSystemAddress("KLAN", address(0xC196247AA267Db0DF216d5385bCD23e5cf25EA6A));
        Cho.AddSystemAddress("SMG", address(0xa8e8412d9B4341239269cBA38ad949fE4870be34));
        Cho.AddSystemAddress("iCE", address(0x2fA079d2dAA29Ec8925484F9E9021e9191fE4aE4));
        Cho.AddSystemAddress("PWA", address(0x5d4cb28eA61125a1fD3c927162C6F1969DD26788));
        Cho.AddSystemAddress("LIT", address(0xf5E3Cc8d22B10d967bE49FE103e496F449C8604E));
        Cho.AddSystemAddress("RZR", address(0x50e40e8555AaB6b9c6CFF691E14070b6F38142Cb));
        Cho.AddSystemAddress("FLT", address(0x86F0985Cd6Ab3196ea8DceBa87B92a2e22124633));
        Cho.AddSystemAddress("Gokuldham", address(0x920401FDce49Fc70A2D4cD70DB0dD90212a97f98));
        Cho.AddSystemAddress("KPOP", address(0x982B52a54916B899c60031772cc85b041613510E));
        Cho.AddSystemAddress("CiA", address(0x2e5898b2e107a3cAf4f0597aCFE5D2e6d73F2196));
        Cho.AddSystemAddress("ACiD", address(0xf8b6e89b851e03c724aad1F5170230A60490b819));
        Cho.AddSystemAddress("Malta", address(0xee62EE9A354E55dF7C39209B4304161369333fF7));
        Cho.AddSystemAddress("ACAB", address(0x241DA2613b0A01C2f60acB636b21A8E082E2f2F0));

        //
        Cho.AddSystemAddress("BillBurr", address(0xF7ebb9bc80fb6395373c6BbDF690fcFfb217a691));

        //
        Cho.AddSystemAddress("Bonus", address(0xB8FaCE58CB05C55BBBA9c564A550cc2402A40b5b));

        ///
        Cho.AddSystemAddress("LEPROSY", address(0x7759A6D283192ef2BA082923d28Bec6eBfAf9D68));

        //
        Cho.AddSystemAddress("PI", address(0xA2262D7728C689526693aE893D0fD8a352C7073C));
        Cho.AddSystemAddress("G5", address(0x2fc636E7fDF9f3E8d61033103052079781a6e7D2));

        //
        Cho.AddSystemAddress("libAtropaMath", address(0xB680F0cc810317933F234f67EB6A9E923407f05D));
        Cho.AddSystemAddress("AFFECTION", address(0x24F0154C1dCe548AdF15da2098Fdd8B8A3B8151D));
        Cho.AddSystemAddress("RESTRAININGORDER", address(0xEf2125f5d1f7A3d68038F27e681258d13a73E718));
        Cho.AddSystemAddress("WITHOUT", address(0x173216Ed67eBF3E6767D86e8b3Ff32e0d64437bF));

        //
        Cho.AddSystemAddress("mariarahel", address(0xD32c39fEE49391c7952d1b30b15921b0D3b42E69));   
    }
}