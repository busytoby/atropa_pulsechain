// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../include/enchantment.sol";
import "../interfaces/16b_zaointerface.sol";

contract CONJURELIB is DYSNOMIA {
    string public constant Type = "ConjureLib";

    ZAOINTERFACE public Zao;
    mapping(uint64 => Enchantment) private _conjures;
    mapping(string => uint64) private _enchantments;
    string[] public KnownEnchantments;
    uint64[10] public Levels = [5040, 40320, 362880, 3628800, 39916800, 479001600, 6227020800, 87178291200, 1307674368000, 20922789888000];

    constructor(address ZaoAddress) DYSNOMIA(unicode"DYSNOMIA ConjureLib", unicode"ConjureLib", address(DYSNOMIA(ZaoAddress).Xiao())) {
        Zao = ZAOINTERFACE(ZaoAddress);
        addOwner(ZaoAddress);
        addOwner(tx.origin);
        Zao.VoidQing().Void().AddLibrary("conjure", address(this));
        _mintToCap();
    }

    function Withdraw(address what, uint256 amount) public onlyOwners {
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
    }

    function EnchantmentCount() public view returns (uint256) {
        return KnownEnchantments.length;
    }

    function Level(uint64 Saat) public view returns (uint8) {
        for(uint8 i = 0; i < 10; i++)
            if(Saat < Levels[i]) return i;
        return 10;
    }

    error NotTokenOwner(address Token, address originRequestor);
    error ApproveAndPay(address TOKEN, address THIS, uint256 AMOUNT);
    function Enchant(address QiAddress, uint64 Rho, uint64 Upsilon, uint64 Ohm, string memory IntendedNoun) public onlyOwners returns (uint64) {
        uint8 Amplifier;
        QIINTERFACE Qi = QIINTERFACE(QiAddress);
        if(!Qi.owner(tx.origin)) revert NotTokenOwner(QiAddress, tx.origin);

        address wmContract = Zao.VoidQing().VAT().Alias(0, "WM");
        bool success = DYSNOMIA(wmContract).transferFrom(tx.origin, address(this), 1 * 10 ** decimals());
        if(!success) revert ApproveAndPay(wmContract, address(this), 1 * 10 ** decimals());

        Amplifier = Level(_enchantments[IntendedNoun] + Upsilon) ** 2;
        Upsilon = (Upsilon * Amplifier);

        if(_conjures[Upsilon].Saat[2] < Rho + Qi.Saat(0)) {
            _conjures[Upsilon].Noun = IntendedNoun;
            _conjures[Upsilon].Saat[2] = Ohm;
        } else if(_conjures[Upsilon].Saat[1] > Upsilon + Qi.Saat(1))
            Rho /= 2;

        UpdateSaat(Qi, Rho + Qi.Saat(0), Upsilon + Qi.Saat(1), Ohm + Qi.Saat(2));

        if(_enchantments[IntendedNoun] == 0) {
            KnownEnchantments.push(IntendedNoun);
            _conjures[Upsilon].Noun = IntendedNoun;
        }
        if(_enchantments[IntendedNoun] < MotzkinPrime)
            _enchantments[IntendedNoun] += Upsilon;
        Qi.mintToCap();
        _mintToCap();
        return Upsilon;
    }

    function Imbue(address QiAddress, uint64 Rho, uint64 Upsilon, uint64 Ohm, string memory IntendedAdjective) public onlyOwners returns (uint64) {
        uint8 Amplifier;
        QIINTERFACE Qi = QIINTERFACE(QiAddress);
        if(!Qi.owner(tx.origin)) revert NotTokenOwner(QiAddress, tx.origin);

        address wmContract = Zao.VoidQing().VAT().Alias(0, "WM");
        bool success = DYSNOMIA(wmContract).transferFrom(tx.origin, address(this), 1 * 10 ** decimals());
        if(!success) revert ApproveAndPay(wmContract, address(this), 1 * 10 ** decimals());

        Amplifier = Level(_enchantments[IntendedAdjective] + Ohm) ** 2;
        Ohm = (Ohm * Amplifier) + Qi.Saat(2);

        if(_conjures[Upsilon].Saat[2] < Rho + Qi.Saat(0)) {
            _conjures[Upsilon].Adjective = IntendedAdjective;
            _conjures[Upsilon].Saat[2] = Ohm;
        } else if(_conjures[Upsilon].Saat[1] > Upsilon + Qi.Saat(1))
            Rho /= 2;

        UpdateSaat(Qi, Rho + Qi.Saat(0), Upsilon + Qi.Saat(1), Ohm + Qi.Saat(2));

        if(_enchantments[IntendedAdjective] == 0) {
            KnownEnchantments.push(IntendedAdjective);
            _conjures[Upsilon].Adjective = IntendedAdjective;
        }
        if(_enchantments[IntendedAdjective] < MotzkinPrime)
            _enchantments[IntendedAdjective] += Ohm;

        Qi.mintToCap();
        _mintToCap();
        return Ohm;
    }

    function UpdateSaat(QIINTERFACE Qi, uint64 Rho, uint64 Upsilon, uint64 Ohm) internal {
        Qi.Modify(Rho, Upsilon, Ohm);
        _mintToCap();
    }

    function RenameLesserQi(address SuperiorQi, uint64 Saat1, string memory Adverb, string memory Noun, string memory Adjective) public {
        if(!DYSNOMIA(SuperiorQi).owner(tx.origin)) revert NotTokenOwner(SuperiorQi, tx.origin);
        QIINTERFACE Qi = QIINTERFACE(SuperiorQi);
        assert(Saat1 < Qi.Saat(1));
        _payToRename(Saat1);

        _conjures[Saat1].Adverb = Adverb;
        _conjures[Saat1].Noun = Noun;
        _conjures[Saat1].Adjective = Adjective;

        _conjures[Saat1].Saat[0] = Qi.Saat(0);
        _conjures[Saat1].Saat[2] = Qi.Saat(2);
    }

    function _payToRename(uint64 Saat) internal {
        if(_conjures[Saat].previousCost == 0) _conjures[Saat].previousCost = 1 * 10 ** decimals();
        address affectionContract = Zao.VoidQing().VAT().Alias(0, "AFFECTION");

        bool success = DYSNOMIA(affectionContract).transferFrom(tx.origin, address(this), _conjures[Saat].previousCost);
        if(!success) revert ApproveAndPay(affectionContract, address(this), _conjures[Saat].previousCost);
        _conjures[Saat].previousCost *= 2;
    }

    function ForceRename(uint64 Saat1, string memory Adverb, string memory Noun, string memory Adjective) public onlyOwners {
        _conjures[Saat1].Adverb = Adverb;
        _conjures[Saat1].Noun = Noun;
        _conjures[Saat1].Adjective = Adjective;
    }

    function RenameQi(string memory Adverb, string memory Noun, string memory Adjective) public onlyOwners {
        if(!DYSNOMIA(msg.sender).owner(tx.origin)) revert NotTokenOwner(msg.sender, tx.origin);
        QIINTERFACE Qi = QIINTERFACE(msg.sender);
        Enchantment memory _p = _conjures[Qi.Saat(1)];
        _payToRename(_p.Saat[1]);

        _p.Adverb = Adverb;
        _p.Noun = Noun;
        _p.Adjective = Adjective;

        _p.Saat[0] = Qi.Saat(0);
        _p.Saat[2] = Qi.Saat(2);

        _conjures[_p.Saat[1]] = _p;
        _mintToCap();
    }

    function qName() public view returns (string memory) {
        Enchantment memory _p = _conjures[QIINTERFACE(msg.sender).Saat(1)];
        string memory _noun;
        string memory _adjective;
        string memory _adverb;
        
        if(bytes(_p.Noun).length == 0) _noun = "Qi";
        else _noun = _p.Noun;
        if(bytes(_p.Adjective).length == 0) _adjective = "";
        else _adjective = string.concat(_p.Adjective, " ");
        if(bytes(_p.Adverb).length == 0) _adverb = "";
        else _adverb = string.concat(_p.Adverb, " ");
        return string.concat(_adverb, _adjective, _noun);
    }

    function qSymbol() public view returns (string memory) {
        Enchantment memory _p = _conjures[QIINTERFACE(msg.sender).Saat(1)];
        if(bytes(_p.Noun).length == 0) return string.concat("QI(", Zao.VoidQing().CYUN().String(_p.Saat[1]), ")");
        return string.concat(_p.Noun,"(",Zao.VoidQing().CYUN().String(_p.Saat[1]), ")");
    }
}