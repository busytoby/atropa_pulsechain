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
    NTI public constant TreasuryMinter = NTI(0xC7bDAc3e6Bb5eC37041A11328723e9927cCf430B);
    NTI public constant FederalMinter = NTI(0xc15c5F699Daf5e1135732139f05D2c05b3EF4354);
    NTI public constant IndexMinter = NTI(0x0c4F73328dFCECfbecf235C9F78A4494a7EC5ddC);
    address public constant FED = address(0x1D177CB9EfEEa49A8B97ab1C72785a3A37ABc9Ff);
    address public BUREAU;
    mapping(address ctx => address owner) public TreasuryTokens;

    constructor() {
        BUREAU = IndexMinter.New("Dysnomian Credit Bureau", unicode"BUREAU ㉾", 0, address(0xC7145e1290B1d1221Aba5Ae48d4aCE17c6BE088F));
    }

    function GetStandardTokenParent(address ctx) public view returns (address) {
        ERC20 NP = TTI(ctx).Parent();
        while(TreasuryTokens[address(NP)] != address(0x0)) NP = TTI(TreasuryTokens[address(NP)]).Parent();
        return address(NP);
    }

    function GetTreasuryTokenOwner(address ctx) public view returns (address) {
        if(TreasuryTokens[ctx] != address(0x0)) return TreasuryTokens[ctx];
        return FederalMinter.GetTreasuryTokenOwner(ctx);
    }

    function New(string calldata Name, string calldata Symbol, uint256 InitialMint) public returns (address) {
        ERC20 BuyToken = ERC20(WMContract);
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), InitialMint);
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        TT _new = new TT(Name, Symbol, InitialMint, address(IndexMinter), BUREAU);
        TreasuryTokens[address(_new)] = address(this);

        return address(_new);
    }
}

interface NTI {
    function GetTreasuryTokenOwner(address ctx) external view returns (address);
    function New(string calldata Name, string calldata Symbol, uint256 InitialMint, address Parent) external returns (address);
}

interface TTI {
    function Parent() external view returns (ERC20);
    function Creator() external view returns (address);
}

contract TT is ERC20, ERC20Burnable {

    NT public constant TreasuryMinter = NT(0xC7bDAc3e6Bb5eC37041A11328723e9927cCf430B);
    NT public constant FederalMinter = NT(0xc15c5F699Daf5e1135732139f05D2c05b3EF4354);
    NT public IndexMinter;
    ERC20 public Parent;
    address public Creator;
    bool public Debenture;
    address public constant _mathlib = address(0xB680F0cc810317933F234f67EB6A9E923407f05D);
    uint64 public _mintingKey;
    mapping(address => uint8) public _hu;
    mapping(address => mapping(address => uint256)) public _ho;

    constructor(string memory Name, string memory Symbol, uint256 InitialMint, address IndexMinterAddress, address ParentAddress) ERC20(Name, Symbol) {
        IndexMinter = NT(IndexMinterAddress);
        Parent = ERC20(ParentAddress);
        Creator = tx.origin;
        atropaMath mathlib = atropaMath(_mathlib);        
        _mintingKey = mathlib.Random();
        _mint(tx.origin, InitialMint);
        _hu[tx.origin] = 255;
        Debenture = true;
    }

    function Multiplier(uint256 addition) public view returns (uint256) {
        return ((addition + totalSupply()) / 1111111111000000000000000000) + 1;
    }

    function ha() public {
        _hu[msg.sender] = 1;
    }

    function ho(address cx, uint256 value) public returns (uint256 _v) {
        if(_hu[msg.sender] < 1) revert FuckOff(tx.origin);
        if(cx != msg.sender && cx != tx.origin && _hu[msg.sender] < 5) revert FuckOff(tx.origin);
        _v = _ho[tx.origin][cx];
        if(_hu[msg.sender] > 10) _ho[tx.origin][cx] = value;
    }

    function withdraw(address token, uint256 value) public {
        address Owner = IndexMinter.TreasuryTokens(address(this));
        address TokenOwner = IndexMinter.TreasuryTokens(token);
        if(token == address(Parent) || Owner != msg.sender || TokenOwner != msg.sender) revert FuckOff(msg.sender);       
        ERC20(token).transfer(msg.sender, value);
        Debenture = false;
    }

    event Recovery(address Client, address Contract, uint256 Amount);
    function Claim(address Contract, uint256 Amount) public {
        if(!Debenture) revert FuckOff(msg.sender);
        if(address(Parent) == TreasuryBillContract) revert FuckOff(msg.sender);
        address Owner = IndexMinter.TreasuryTokens(Contract);
        if(Owner == address(0x0)) revert FuckOff(msg.sender);
        if(Creator != TTI(Contract).Creator()) revert FuckOff(msg.sender);
        if(address(Parent) != address(TTI(Contract).Parent())) revert FuckOff(msg.sender);
        ERC20 BuyToken = ERC20(Contract);
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), Amount);
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        if(Contract == address(this)) ERC20Burnable(Contract).burn(Amount);
        Parent.transfer(msg.sender, Amount);
        emit Recovery(msg.sender, Contract, Amount);
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
