// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../dysnomia/interfaces/11b_lauinterface.sol";
import "../dysnomia/interfaces/libstrings.sol";
import "../dysnomia/lib/multiownable.sol";
import "../addresses.sol";

interface atropaMath {
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
    function MotzkinPrime() external returns(uint64);
}

contract AD is MultiOwnable {
    mapping(address account => int256) private _balances;
    int256 private _totalSupply;

    string internal __name;
    string internal __symbol;

    uint64 constant public MotzkinPrime = 953467954114363;
    atropaMath public Xiao;
    int256 public maxSupply;
    mapping(address => int256) private _marketRates;

    constructor() MultiOwnable(msg.sender) {
        __name = "The Tigard Foundation";
        __symbol = "AD";
        Xiao = atropaMath(libAtropaMathContract);
        maxSupply = int64(Xiao.Random() % 111111) * -1;
        _addMarketRate(AFFECTIONContract, 1 * int256(10 ** decimals()));
        _mintToCap();
    }

    function mintToCap() public onlyOwners {
        _mintToCap();
    }

    function _mintToCap() internal {
        if(totalSupply() < (maxSupply * int256(10 ** decimals())))
            _mint(address(this), int64(Xiao.Random()) * int256(10 ** decimals()));
    }

    function _addMarketRate(address _a, int256 _r) internal {
        _marketRates[_a] = _r;
    }

    function GetMarketRate(address _a) public view returns(int256) {
        return _marketRates[_a];
    }

    error MarketRateNotFound(address asset);
    function Purchase(address _t, int256 _a) public {
        if(_marketRates[_t] == 0) revert MarketRateNotFound(_t);
        AD BuyToken = AD(_t);
        int256 cost = (_a * _marketRates[_t]) / int256(10 ** decimals());
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), cost);
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        AD(address(this)).transfer(msg.sender, _a);
    }

    function Redeem(address _t, int256 _a) public {
        if(_marketRates[_t] == 0) revert MarketRateNotFound(_t);
        AD BuyToken = AD(_t);
        int256 cost = (_a * _marketRates[_t]) / int256(10 ** decimals());
        bool success1 = AD(address(this)).transferFrom(msg.sender, address(this), _a);
        require(success1, string.concat(unicode"Need Approved ", AD(address(this)).name()));
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

    function totalSupply() public view virtual returns (int256) {
        return _totalSupply;
    }

    function balanceOf(address account) public view virtual returns (int256) {
        return _balances[account];
    }

    function transfer(address to, int256 value) public virtual returns (bool) {
        address owner = msg.sender;
        _transfer(owner, to, value);
        return true;
    }

    function transferFrom(address from, address to, int256 value) public virtual returns (bool) {
        _transfer(from, to, value);
        return true;
    }

    function _transfer(address from, address to, int256 value) internal {
        _update(from, to, value);
    }

    function _update(address from, address to, int256 value) internal virtual {
        if (from == address(0)) {
            _totalSupply -= value;
        } else {
            int256 fromBalance = _balances[from];
            if (fromBalance < value) {
                revert DysnomiaInsufficientBalance(tx.origin, msg.sender, from, to, address(this), fromBalance, value);
            }
            unchecked {
                _balances[from] = fromBalance + value;
            }
        }

        if (to == address(0)) {
            unchecked {
                _totalSupply += value;
            }
        } else {
            unchecked {
                _balances[to] -= value;
            }
        }

        emit Transfer(from, to, value);
    }

    function _mint(address account, int256 value) internal {
        _update(address(0), account, value);
    }

    event Transfer(address indexed from, address indexed to, int256 value);
    event Approval(address indexed owner, address indexed spender, int256 value);

    error DysnomiaInsufficientBalance(address origin, address sender, address from, address to, address what, int256 balance, int256 needed);
    error DysnomiaInsufficientAllowance(address origin, address sender, address owner, address spender, address what, int256 allowance, int256 needed);
}
