// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "00c_multiownable.sol";
import "addresses.sol";

interface atropaMath {   
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
}

abstract contract SH is ERC20, ERC20Burnable, MultiOwnable {
    uint64 constant public MotzkinPrime = 953467954114363;
    atropaMath public Xiao;
    uint256 public maxSupply;
    address[] private Addresses;
    mapping(address => uint256) public MarketRates;
    uint256 public Reserve;

    constructor(address mathContract, uint256 _maxSupply) {
        Xiao = atropaMath(mathContract);
        maxSupply = _maxSupply - 1;
        SetReservePercentage(100);
        AddMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
    }

    //mapping(uint256 => mapping(uint256 => Fei)) internal Rho;
    //mapping(uint256 => Bao) internal Psi;

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

    function KnownAddresses() public view returns(address[] memory) {
        return Addresses;
    }

    function KnownAddress(address _a) public view returns(bool) {
        for(uint256 i = 0; i < Addresses.length; i++)
            if(Addresses[i] == _a) return true;
        return false;
    }

    function RemoveAddress(address _a) public onlyOwners {        
        int256 foundidx = -1;
        for(uint256 i = 0; i < Addresses.length && foundidx < 0; i++)
            if(Addresses[i] == _a) foundidx = int256(i);
        if(foundidx >= 0) {
            Addresses[uint256(foundidx)] = Addresses[Addresses.length - 1];
            Addresses.pop();
        }
    }

    function RegisterAddress(address _a) public onlyOwners {
        if(_a != address(this) && !KnownAddress(_a)) Addresses.push(_a);
    }

    event LogEvent(string, bytes);
    function Log(string calldata log, bytes calldata data) internal {
        emit LogEvent(log, data);
        mintToCap();
    }
}