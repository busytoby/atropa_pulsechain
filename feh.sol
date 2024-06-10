// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";
import "fan.sol";
import "fei.sol";

interface atropaMath {   
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
}

abstract contract Feh is ERC20, ERC20Burnable, Ownable {
    uint64 constant public MotzkinPrime = 953467954114363;
    //atropaMath internal Xiao = atropaMath(libAtropaMathContract);
    atropaMath internal Xiao = atropaMath(0xDf159010A8d1B173262EBb3D7b5393Dc0333301d);

    mapping(address => Fan) internal Rho;
    mapping(uint256 => mapping(uint256 => Fei)) internal Psi;

    ERC20 internal constant DaiToken = ERC20(dai);
    ERC20 internal USDCToken = ERC20(usdc);
    ERC20 internal USDTToken = ERC20(usdt);
    ERC20 internal G5Token = ERC20(G5Contract);
    ERC20 internal PIToken = ERC20(PIContract); // trying to remove these

    function _mintToCap() internal {
        if(totalSupply() <= (1111111111 * 10 ** decimals()))
            _mint(address(this), 1 * 10 ** decimals());
    }

    function toBytes(uint256 x) internal pure returns (bytes memory b) { 
        b = new bytes(32); 
        assembly { 
            mstore(xor(b, 32), x) 
        } 
    }

    function toBytes(string memory x) internal pure returns (bytes memory b) { 
        b = bytes(x);
        assert(b.length < 32);
    } 

    function BuyWithG5(uint256 amount) public {
        bool success1 = G5Token.transferFrom(msg.sender, address(this), (amount / 5));
        require(success1, unicode"Need Approved Gimme5");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function BuyWithPI(uint256 amount) public {
        bool success1 = PIToken.transferFrom(msg.sender, address(this), (amount / 300));
        require(success1, unicode"Need Approved pINDEPENDENCE");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function BuyWithMATH(uint256 amount) public {
        bool success1 = ERC20(libAtropaMathContract).transferFrom(msg.sender, address(this), amount);
        require(success1, unicode"Need Approved MATH");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function BuyWithDAI(uint256 amount) public {
        bool success1 = DaiToken.transferFrom(msg.sender, address(this), amount);
        require(success1, unicode"Need Approved DAI");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function BuyWithUSDC(uint256 amount) public {
        uint256 _a = (amount / (10**decimals())) * 10 ** USDCToken.decimals();
        bool success1 = USDCToken.transferFrom(msg.sender, address(this), _a);
        require(success1, unicode"Need Approved USDC");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function BuyWithUSDT(uint256 amount) public {
        uint256 _a = (amount / (10**decimals())) * 10 ** USDTToken.decimals();
        bool success1 = USDTToken.transferFrom(msg.sender, address(this), _a);
        require(success1, unicode"Need Approved USDT");
        ERC20(address(this)).transfer(msg.sender, amount);
    }
}