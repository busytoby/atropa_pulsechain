// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";

interface RNG {
    function Generate() external returns(uint64);
}

contract atropaMath is ERC20, ERC20Burnable, Ownable {
    uint64 constant public MotzkinPrime = 953467954114363;

    RNG private RandomNumberGeneratorToken;

    ERC20 private DaiToken;
    ERC20 private USDCToken;
    ERC20 private USDTToken;
    ERC20 private G5Token;
    ERC20 private PIToken;

    constructor() ERC20(/*name short=*/ unicode"libAtropaMath v1.1", /*symbol long=*/ unicode"MATH") Ownable(msg.sender) {
        DaiToken = ERC20(dai);
        USDCToken = ERC20(usdc);
        USDTToken = ERC20(usdt);
        G5Token = ERC20(G5Contract);
        PIToken = ERC20(PIContract);

        //RandomNumberGeneratorToken = RNG(0xa96BcbeD7F01de6CEEd14fC86d90F21a36dE2143);
        RandomNumberGeneratorToken = RNG(0x751b8744dc56889a5d9b2F9802e117BEcF029748);

        _mint(address(this), 1 * 10 ** decimals());
    }

    function Random() public returns(uint64) {
        if(totalSupply() <= (1111111111 * 10 ** decimals()))
            _mint(address(this), 1 * 10 ** decimals());
        return RandomNumberGeneratorToken.Generate();
    }

    function hashWith(address a, address b) public pure returns (uint256 hash) {        
        hash = 0;
        uint160 _a = uint160(a);
        uint160 _b = uint160(b) / 15;
        unchecked {
            while(hash == 0) {
                hash = (_a**_b)%MotzkinPrime;
                _b = _b/2;
            }
        }
        //return modExp(uint256(uint160(a)), uint256(uint160(b)), MotzkinPrime);
    }

    function modExp64(uint64 _b, uint64 _e, uint64 _m) public returns(uint64 result) {
        uint256 B = _b;
        uint256 E = _e;
        uint256 M = _m;
        uint64 R = uint64(modExp(B, E, M) % 18446744073709551615);
        return R;
    }

    function modExp(uint256 _b, uint256 _e, uint256 _m) public returns (uint256 result) {
        assembly {
            // Free memory pointer
            let pointer := mload(0x40)

            // Define length of base, exponent and modulus. 0x20 == 32 bytes
            mstore(pointer, 0x20)
            mstore(add(pointer, 0x20), 0x20)
            mstore(add(pointer, 0x40), 0x20)

            // Define variables base, exponent and modulus
            mstore(add(pointer, 0x60), _b)
            mstore(add(pointer, 0x80), _e)
            mstore(add(pointer, 0xa0), _m)

            // Store the result
            let value := mload(0xc0)

            // Call the precompiled contract 0x05 = bigModExp
            if iszero(call(not(0), 0x05, 0, pointer, 0xc0, value, 0x20)) {
                revert(0, 0)
            }

            result := mload(value)
        }
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

    function BuyWithG5(uint256 amount) public {
        bool success1 = G5Token.transferFrom(msg.sender, address(this), (amount / 4));
        require(success1, unicode"Need Approved Gimme5");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function BuyWithPI(uint256 amount) public {
        bool success1 = PIToken.transferFrom(msg.sender, address(this), (amount / 212));
        require(success1, unicode"Need Approved pINDEPENDENCE");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function BuyWithMATH(uint256 amount) public {
        bool success1 = ERC20(0x5EF3011243B03f817223A19f277638397048A0DC).transferFrom(msg.sender, address(this), amount);
        require(success1, unicode"Need Approved MATH");
        ERC20(address(this)).transfer(msg.sender, amount);
    }
}