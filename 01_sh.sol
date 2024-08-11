// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "000_dysnomia.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "00c_multiownable.sol";
import "addresses.sol";

interface atropaMath {
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
}

abstract contract SH is DYSNOMIA, MultiOwnable {
    uint64 constant public MotzkinPrime = 953467954114363;
    atropaMath public Xiao;
    uint256 public maxSupply;
    mapping(address => uint256) public MarketRates;
    uint256 public Reserve;

    constructor(address mathContract, uint256 _maxSupply) {
        Xiao = atropaMath(mathContract);
        maxSupply = _maxSupply - 1;
        SetReservePercentage(100);
        AddMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
    }

    function Rename(string memory newName, string memory newSymbol) public onlyOwners {
        _name = newName;
        _symbol = newSymbol;
    }

    function name() public view override returns (string memory) {
        return _name;
    }

    function symbol() public view override returns (string memory) {
        return _symbol;
    }

    function mintToCap() public onlyOwners {
        if(totalSupply() < (maxSupply * 10 ** decimals()))
            _mint(address(this), 1 * 10 ** decimals());
    }

    function AddMarketRate(address _a, uint256 _r) public onlyOwners {
        MarketRates[_a] = _r;
    }

    function SetReservePercentage(uint8 _p) public onlyOwners {
        assert(_p <= 100);
        Reserve = maxSupply * _p * 10 ** (decimals() - 2);
    }

    function Purchase(address _t, uint256 _a) public {
        assert(MarketRates[_t] > 0);
        if(ERC20(address(this)).balanceOf(address(this)) < Reserve) assert(owner(msg.sender));
        ERC20 BuyToken = ERC20(_t);
        bool success1 = BuyToken.transferFrom(msg.sender, address(this), ((_a / (10 ** decimals())) * MarketRates[_t]));
        require(success1, string.concat(unicode"Need Approved ", BuyToken.name()));
        ERC20(address(this)).transfer(msg.sender, _a);
    }

/*
    event LogEvent(string, bytes);
    function Log(string calldata log, bytes calldata data) internal {
        emit LogEvent(log, data);
        mintToCap();
    }
*/
}