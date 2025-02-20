// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "./addresses.sol";

interface atropaMath {
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
    function MotzkinPrime() external returns(uint64);
}

contract NT {
    NT public constant TreasuryMinter = NT(0xC7bDAc3e6Bb5eC37041A11328723e9927cCf430B);
    NT public constant IndexMinter = NT(0x0c4F73328dFCECfbecf235C9F78A4494a7EC5ddC);
    address public immutable NOTS;
    address public immutable SKILLS;
    address public immutable NINE;
    mapping(address ctx => address owner) public TreasuryTokens;

    address public constant BBC = address(0x8b4cfb020aF9AcAd95AD80020cE8f67FBB2C700E);

    constructor() {
        TT _new = new TT("N㉾tsD㉾tQ", unicode"NoSDoQ", 1000000000000000000, address(this), address(0x1D177CB9EfEEa49A8B97ab1C72785a3A37ABc9Ff));
        TT _new2 = new TT("Skill", unicode"SKILL", 1111111111000000000000000000, address(this), address(WMContract));
        TT _new3 = new TT("Zuts aLARP", unicode"LARP", 1111111111000000000000000000, address(this), address(_new2));       
        TreasuryTokens[address(_new3)] = tx.origin;
        TT _newB = new TT("Nine", unicode"9", 9000000000000000000, address(this), address(BBC));
        TreasuryTokens[address(_newB)] = tx.origin;
        /*
        _newB = new TT("Eight", unicode"8", 8000000000000000000, address(this), address(_newB));
        TreasuryTokens[address(_newB)] = tx.origin;
        _newB = new TT("Seven", unicode"7", 7000000000000000000, address(this), address(_newB));
        TreasuryTokens[address(_newB)] = tx.origin;
        _newB = new TT("Six", unicode"6", 6000000000000000000, address(this), address(_newB));
        TreasuryTokens[address(_newB)] = tx.origin;
        _newB = new TT("Five", unicode"5", 5000000000000000000, address(this), address(_newB));
        TreasuryTokens[address(_newB)] = tx.origin;
        _newB = new TT("Four", unicode"4", 4000000000000000000, address(this), address(_newB));
        TreasuryTokens[address(_newB)] = tx.origin;
        _newB = new TT("Three", unicode"3", 3000000000000000000, address(this), address(_newB));
        TreasuryTokens[address(_newB)] = tx.origin;
        _newB = new TT("Two", unicode"2", 2000000000000000000, address(this), address(_newB));
        TreasuryTokens[address(_newB)] = tx.origin;
        _newB = new TT("One", unicode"1", 1000000000000000000, address(this), address(_newB));
        TreasuryTokens[address(_newB)] = tx.origin;
        */
        NOTS = address(_new);
        SKILLS = address(_new2);
        NINE = address(_newB);
        TreasuryTokens[NOTS] = tx.origin;
        TreasuryTokens[SKILLS] = tx.origin;

        _new = new TT("SEMIOTIC", unicode"❄️", 131000000000000000000, address(this), address(AtropaContract));
    }

    function GetStandardTokenParent(address ctx) public view returns (address) {
        ERC20 NP = TTI(ctx).Parent();
        while(TreasuryTokens[address(NP)] != address(0x0)) NP = TTI(TreasuryTokens[address(NP)]).Parent();
        return address(NP);
    }

    function GetTreasuryTokenOwner(address ctx) public view returns (address) {
        if(TreasuryTokens[ctx] != address(0x0)) return TreasuryTokens[ctx];
        return IndexMinter.GetTreasuryTokenOwner(ctx);
    }

    function Transfer(address ctx, address newOwner) public {
        if(TreasuryTokens[ctx] == tx.origin)
            TreasuryTokens[ctx] = newOwner;
    }

    function NewGai(string calldata Name, string calldata Symbol) public returns (address) {
        ERC20 BuyToken = ERC20(WMContract);
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), 15000000000000000000);
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        TT _new = new TT(string.concat(Name, unicode" ➎"), string.concat(Symbol, unicode"➎"), 5000000000000000000, address(this), GaiContract);
        TreasuryTokens[address(_new)] = tx.origin;
        TT _new = new TT(string.concat(Name, unicode" ➍"), string.concat(Symbol, unicode"➍"), 5000000000000000000, address(this), GaiContract);
        TreasuryTokens[address(_new)] = tx.origin;
        TT _new = new TT(string.concat(Name, unicode" ➌"), string.concat(Symbol, unicode"➌"), 5000000000000000000, address(this), GaiContract);
        TreasuryTokens[address(_new)] = tx.origin;
        TT _new = new TT(string.concat(Name, unicode" ➋"), string.concat(Symbol, unicode"➋"), 5000000000000000000, address(this), GaiContract);
        TreasuryTokens[address(_new)] = tx.origin;
        TT _new = new TT(string.concat(Name, unicode" ➊"), string.concat(Symbol, unicode"➊"), 5000000000000000000, address(this), GaiContract);
        TreasuryTokens[address(_new)] = tx.origin;

        return address(_new);
    }

    function New(string calldata Name, string calldata Symbol, uint256 InitialMint, address Parent) public returns (address) {
        ERC20 BuyToken = ERC20(WMContract);
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), InitialMint);
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        TT _new = new TT(Name, Symbol, InitialMint, address(this), Parent);

        TreasuryTokens[address(_new)] = tx.origin;

        return address(_new);
    }
}

interface TTI {
    function Parent() external view returns (ERC20);
    function Creator() external view returns (address);
    function publish() external;
}

contract TT is ERC20, ERC20Burnable {
    NT public constant IndexMinter = NT(0x0c4F73328dFCECfbecf235C9F78A4494a7EC5ddC);
    NT public immutable PersonalMinter;
    ERC20 public immutable Parent;
    address public immutable Creator;
    uint256 public immutable Mint;
    address public constant _mathlib = address(0xB680F0cc810317933F234f67EB6A9E923407f05D);
    uint64 public immutable _mintingKey;
    mapping(address => uint8) public _hu;
    mapping(address => mapping(address => uint256[])) public _ho;

    constructor(string memory Name, string memory Symbol, uint256 InitialMint, address IndexMinterAddress, address ParentAddress) ERC20(Name, Symbol) {
        PersonalMinter = NT(IndexMinterAddress);
        Parent = ERC20(ParentAddress);
        Creator = tx.origin;
        atropaMath mathlib = atropaMath(_mathlib);        
        _mintingKey = mathlib.Random();
        _mint(tx.origin, InitialMint);
        _hu[tx.origin] = 255;
        Mint = InitialMint;
    }

    function Multiplier(uint256 addition) public view returns (uint256) {
        return ((addition + totalSupply()) / Mint) + 1;
    }

    function ha() public {
        _hu[msg.sender] = 1;
    }

    function ho(address cx, uint256 value) public returns (uint256 length) {
        if(_hu[msg.sender] < 1) revert FuckOff(tx.origin);
        if(cx != msg.sender && cx != tx.origin && _hu[msg.sender] < 5) revert FuckOff(tx.origin);
        if(value > 0 && _hu[msg.sender] > 10) _ho[tx.origin][cx].push(value);
        length = _ho[tx.origin][cx].length;
    }

    function withdraw(address token, uint256 value) public {
        address Owner = PersonalMinter.TreasuryTokens(address(this));
        if(token == address(Parent) || Owner != msg.sender) revert FuckOff(msg.sender);       
        ERC20(token).transfer(msg.sender, value);
    }

    event Recovery(address Client, address Contract, uint256 Amount);
    function Claim(uint256 Amount) public {
        if(address(Parent) == TreasuryBillContract) revert FuckOff(msg.sender);
        ERC20 BuyToken = ERC20(address(this));
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), Amount);
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        ERC20Burnable(address(this)).burn(Amount);
        Parent.transfer(msg.sender, Amount);
        emit Recovery(msg.sender, address(this), Amount);
    }

    function mint(uint256 amount) public { 
        bool success1 = Parent.transferFrom(msg.sender, address(this), amount * Multiplier(amount));
        require(success1, string.concat(unicode"Need Approved ", Parent.name()));
        _mint(msg.sender, amount);
    }

    function hu(address h, uint8 allow) public {
        if(_hu[tx.origin] <= _hu[h] || _hu[tx.origin] <= allow) revert FuckOff(tx.origin);
        _hu[h] = allow;
    }

    event TTDATA(bytes);
    fallback(bytes calldata data) external payable returns (bytes memory) {
        if(_hu[msg.sender] < 5) revert FuckOff(tx.origin);
        emit TTDATA(data);
        return data;
    }

    error FuckOff(address you);
    receive() external payable {
        if(_hu[msg.sender] < 5) revert FuckOff(tx.origin);
    }
}
