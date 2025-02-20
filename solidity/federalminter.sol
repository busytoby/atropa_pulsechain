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
    NT public constant V1Minter = NT(0xC7bDAc3e6Bb5eC37041A11328723e9927cCf430B);
    address public constant FED = address(0x1D177CB9EfEEa49A8B97ab1C72785a3A37ABc9Ff);
    address public FDIC;
    mapping(address ctx => address owner) public TreasuryTokens;

    constructor() {
        TT _new = new TT("Federal Deposit Insurance Contract", unicode"FD㉾C", 1, address(this), FED);
        FDIC = address(_new);
        TreasuryTokens[FDIC] = tx.origin;
        _new.publish();
    }

    function GetTreasuryTokenOwner(address ctx) public view returns (address) {
        if(TreasuryTokens[ctx] != address(0x0)) return TreasuryTokens[ctx];
        return V1Minter.TreasuryTokens(ctx);
    }

    function Transfer(address ctx, address newOwner) public {
        if(TreasuryTokens[ctx] == tx.origin)
            TreasuryTokens[ctx] = newOwner;
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
    function Debenture() external returns (bool);
}

contract TT is ERC20, ERC20Burnable {

    NT public V2Minter;
    ERC20 public Parent;
    bool public Debenture;
    address public constant FED = address(0x1D177CB9EfEEa49A8B97ab1C72785a3A37ABc9Ff);
    address public constant _mathlib = address(0xB680F0cc810317933F234f67EB6A9E923407f05D);
    uint64 public _mintingKey;
    mapping(address => uint8) public _hu;
    mapping(address => mapping(address => uint256)) public _ho;

    constructor(string memory Name, string memory Symbol, uint256 InitialMint, address V2MinterAddress, address ParentAddress) ERC20(Name, Symbol) {
        V2Minter = NT(V2MinterAddress);
        if(ParentAddress != FED) {
            address ParentOwner = V2Minter.GetTreasuryTokenOwner(ParentAddress);
            if(ParentOwner == address(0x0)) revert FuckOff(msg.sender);
        }
        Parent = ERC20(ParentAddress);
        atropaMath mathlib = atropaMath(_mathlib);        
        _mintingKey = mathlib.Random();
        _mint(tx.origin, InitialMint);
        _hu[tx.origin] = 255;
        Debenture = true;
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

    event Published(address);
    function publish() public {
        if(_hu[tx.origin] < 100) revert FuckOff(tx.origin);
        Debenture = false;
        emit Published(address(this));
    }

    function withdraw(address token, uint256 value) public {
        if(Debenture) revert FuckOff(tx.origin);
        address Owner = V2Minter.TreasuryTokens(address(this));
        address TokenOwner = V2Minter.TreasuryTokens(token);
        if(TokenOwner == address(0x0))
            TokenOwner = V2Minter.GetTreasuryTokenOwner(token);
        else if(TTI(token).Debenture()) revert FuckOff(tx.origin);
        if(token == address(Parent) || Owner != msg.sender || TokenOwner == address(0x0)) revert FuckOff(msg.sender);       
        ERC20(token).transfer(msg.sender, value);
    }

    event Recovery(address Client, address Contract, uint256 Amount);
    function Claim(address Contract, uint256 Amount) public {
        address Owner = V2Minter.TreasuryTokens(Contract);
        if(Owner == address(0x0)) {
            Owner = V2Minter.GetTreasuryTokenOwner(Contract);
            if(Owner == address(0x0)) revert FuckOff(msg.sender);
        } else if(!TTI(Contract).Debenture()) revert FuckOff(msg.sender);
        ERC20 BuyToken = ERC20(Contract);
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), Amount);
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        if(Contract == address(this)) ERC20Burnable(Contract).burn(Amount);
        Parent.transfer(msg.sender, Amount);
        emit Recovery(msg.sender, Contract, Amount);
    }

    function mint(uint256 amount) public { 
        bool success1 = Parent.transferFrom(msg.sender, address(this), amount);
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
