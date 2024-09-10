// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./interfaces/atropaMath.sol";
import "./interfaces/11b_lauinterface.sol";
import "./interfaces/libstrings.sol";
import "./lib/multiownable.sol";
import "../addresses.sol";

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
        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
    }

    function _addLibraryOwner(VOID Void, string memory what) internal {
        return addOwner(Void.GetLibraryAddress(what));
    }

    function Rename(string memory newName, string memory newSymbol) public virtual onlyOwners {
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

    function _addMarketRate(address _a, uint256 _r) internal {
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
            _totalSupply += value;
        } else {
            uint256 fromBalance = _balances[from];
            if (fromBalance < value) {
                revert DysnomiaInsufficientBalance(tx.origin, msg.sender, from, to, address(this), fromBalance, value);
            }
            unchecked {
                _balances[from] = fromBalance - value;
            }
        }

        if (to == address(0)) {
            unchecked {
                _totalSupply -= value;
            }
        } else {
            unchecked {
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
}
