// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./lib/multiownable.sol";
import "../addresses.sol";

interface atropaMath {
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
    function MotzkinPrime() external returns(uint64);
}

abstract contract DYSNOMIA is MultiOwnable {
    mapping(address account => uint256) private _balances;

    mapping(address account => mapping(address spender => uint256)) private _allowances;

    uint256 private _totalSupply;

    string internal __name;
    string internal __symbol;

    uint64 constant public MotzkinPrime = 953467954114363;
    atropaMath public Xiao;
    uint256 public maxSupply;
    mapping(address => uint256) private _marketRates;

    constructor(string memory name_, string memory symbol_, address mathContract) MultiOwnable(msg.sender) {
        __name = name_;
        __symbol = symbol_;
        Xiao = atropaMath(mathContract);
        maxSupply = Xiao.Random() % 111111;
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
        AddMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
    }

    function Rename(string memory newName, string memory newSymbol) public onlyOwners {
        __name = newName;
        __symbol = newSymbol;
    }

    function mintToCap() public onlyOwners {
        _mintToCap();
    }

    function _mintToCap() internal {
        if(totalSupply() < (maxSupply * 10 ** decimals()))
            _mint(address(this), 1 * 10 ** decimals());
    }

    function AddMarketRate(address _a, uint256 _r) internal {
        _marketRates[_a] = _r;
    }

    function GetMarketRate(address _a) public view returns(uint256) {
        return _marketRates[_a];
    }

    error MarketRateNotFound(address asset);
    function Purchase(address _t, uint256 _a) public {
        if(_marketRates[_t] == 0) revert MarketRateNotFound(_t);
        DYSNOMIA BuyToken = DYSNOMIA(_t);
        uint256 cost = (_a * _marketRates[_t]) / (10 ** decimals());
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), cost);
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        DYSNOMIA(address(this)).transfer(msg.sender, _a);
    }

    function Redeem(address _t, uint256 _a) public {
        if(_marketRates[_t] == 0) revert MarketRateNotFound(_t);
        DYSNOMIA BuyToken = DYSNOMIA(_t);
        uint256 cost = (_a * _marketRates[_t]) / (10 ** decimals());
        bool success1 = DYSNOMIA(address(this)).transferFrom(msg.sender, address(this), _a);
        require(success1, string.concat(unicode"Need Approved ", DYSNOMIA(address(this)).name()));
        BuyToken.transfer(msg.sender, cost);
    }

    function name() public view virtual returns (string memory) {
        return __name;
    }

    function symbol() public view virtual returns (string memory) {
        return __symbol;
    }

    function decimals() public view virtual returns (uint8) {
        return 18;
    }

    function totalSupply() public view virtual returns (uint256) {
        return _totalSupply;
    }

    function balanceOf(address account) public view virtual returns (uint256) {
        return _balances[account];
    }

    function transfer(address to, uint256 value) public virtual returns (bool) {
        address owner = msg.sender;
        _transfer(owner, to, value);
        return true;
    }

    function allowance(address owner, address spender) public view virtual returns (uint256) {
        return _allowances[owner][spender];
    }

    function approve(address spender, uint256 value) public virtual returns (bool) {
        address owner = msg.sender;
        _approve(owner, spender, value);
        return true;
    }

    function transferFrom(address from, address to, uint256 value) public virtual returns (bool) {
        address spender = msg.sender;
        _spendAllowance(from, spender, value);
        _transfer(from, to, value);
        return true;
    }

    function _transfer(address from, address to, uint256 value) internal {
        _update(from, to, value);
    }

    function _update(address from, address to, uint256 value) internal virtual {
        if (from == address(0)) {
            // Overflow check required: The rest of the code assumes that totalSupply never overflows
            _totalSupply += value;
        } else {
            uint256 fromBalance = _balances[from];
            if (fromBalance < value) {
                revert DysnomiaInsufficientBalance(tx.origin, msg.sender, from, to, address(this), fromBalance, value);
            }
            unchecked {
                // Overflow not possible: value <= fromBalance <= totalSupply.
                _balances[from] = fromBalance - value;
            }
        }

        if (to == address(0)) {
            unchecked {
                // Overflow not possible: value <= totalSupply or value <= fromBalance <= totalSupply.
                _totalSupply -= value;
            }
        } else {
            unchecked {
                // Overflow not possible: balance + value is at most totalSupply, which we know fits into a uint256.
                _balances[to] += value;
            }
        }

        emit Transfer(from, to, value);
    }

    function _mint(address account, uint256 value) internal {
        _update(address(0), account, value);
    }

    function _approve(address owner, address spender, uint256 value) internal {
        _approve(owner, spender, value, true);
    }

    function _approve(address owner, address spender, uint256 value, bool emitEvent) internal virtual {
        _allowances[owner][spender] = value;
        if (emitEvent) {
            emit Approval(owner, spender, value);
        }
    }

    function _spendAllowance(address owner, address spender, uint256 value) internal virtual {
        uint256 currentAllowance = allowance(owner, spender);
        if (currentAllowance != type(uint256).max) {
            if (currentAllowance < value) {
                revert DysnomiaInsufficientAllowance(tx.origin, msg.sender, owner, spender, address(this), currentAllowance, value);
            }
            unchecked {
                _approve(owner, spender, currentAllowance - value, false);
            }
        }
    }

    event Transfer(address indexed from, address indexed to, uint256 value);
    event Approval(address indexed owner, address indexed spender, uint256 value);

    error DysnomiaInsufficientBalance(address origin, address sender, address from, address to, address what, uint256 balance, uint256 needed);
    error DysnomiaInsufficientAllowance(address origin, address sender, address owner, address spender, address what, uint256 allowance, uint256 needed);

    function log10(uint256 value) internal pure returns (uint256) {
        uint256 result = 0;
        unchecked {
            for(uint8 _e = 64; _e >= 1; _e /= 2) {
                if (value >= 10 ** _e) {
                    value /= 10 ** _e;
                    result += _e;
                }
            }
        }
        return result;
    }

    function String(uint256 value) internal pure returns (string memory buffer) {
        unchecked {
            uint256 length = log10(value) + 1;
            buffer = new string(length);
            uint256 ptr;
            assembly { ptr := add(buffer, add(32, length)) }
            while (true) {
                ptr--;
                assembly { mstore8(ptr, byte(mod(value, 10), "0123456789abcdef")) }
                value /= 10;
                if (value == 0) break;
            }
        }
    }

    function Hex(address account) internal pure returns(string memory) {
        return Hex(abi.encodePacked(account));
    }

    function Hex(uint256 value) internal pure returns(string memory) {
        return Hex(abi.encodePacked(value));
    }

    function Hex(bytes32 value) internal pure returns(string memory) {
        return Hex(abi.encodePacked(value));
    }

    function Hex(bytes memory data) internal pure returns(string memory) {
        bytes memory hexa = "0123456789abcdef";

        bytes memory str = new bytes(2 + data.length * 2);
        str[0] = "0";
        str[1] = "x";
        for (uint i = 0; i < data.length; i++) {
            str[2+i*2] = hexa[uint(uint8(data[i] >> 4))];
            str[3+i*2] = hexa[uint(uint8(data[i] & 0x0f))];
        }
        return string(str);
    }
}
