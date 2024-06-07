// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";
import "dynamic.sol";

contract Affection is ERC20, ERC20Burnable, Ownable, Dynamic {
    ERC20 private DaiToken;
    ERC20 private USDCToken;
    ERC20 private USDTToken;
    ERC20 private G5Token;
    ERC20 private PIToken;

    constructor() ERC20(/*name short=*/ unicode"AFFECTION™", /*symbol long=*/ unicode"Ⓐ") Ownable(msg.sender) {
        DaiToken = ERC20(dai);
        USDCToken = ERC20(usdc);
        USDTToken = ERC20(usdt);
        G5Token = ERC20(G5Contract);
        PIToken = ERC20(PIContract);

        NewDynamic(aa.Random(), aa.Random(), aa.Random());
        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Upsilon(Mu.Upsilon, false);
        Pi();
        Rho();
        Upsilon(Mu.Upsilon, true);

        _mint(address(this), 1 * 10 ** decimals());
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

    function BuyWithFa(uint256 amount) public {
        bool success1 = ERC20(libConjectureContract).transferFrom(msg.sender, address(this), amount * 4);
        require(success1, unicode"Need Approved Fa");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function BuyWithFaung(uint256 amount) public {
        bool success1 = ERC20(libDynamicContract).transferFrom(msg.sender, address(this), amount * 2);
        require(success1, unicode"Need Approved Faung");
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function Alpha(uint64 _a) public {
        Charge(_a);
        assert(Mu.Sigma > 4);
        Induce();
        Torque();
        Amplify();
        Sustain();
        React();
        _mintToCap();
    }

    function Beta(uint64 _b) public {
        Torque(Mu.Rod, _b);
        Amplify(Mu.Rod, Mu.Rod.Alpha);
        Sustain(Mu.Rod, Mu.Rod.Alpha);
        React(Mu.Rod, Mu.Rod.Alpha, Mu.Cone.Dynamo);
        React(Mu.Cone, Mu.Rod.Alpha, Mu.Rod.Dynamo);
        _mintToCap();
    }

    function Upsilon(uint64 _a, bool Phi) public {
        Mu.Upsilon = Phi ? _a ^ Mu.Ohm ^ Mu.Pi : _a ^ Mu.Ohm;
        _mintToCap();
    }

    function Pi() public {
        Torque(Mu.Cone, Mu.Rod.Kappa);
        Amplify(Mu.Cone, Mu.Cone.Alpha);
        Sustain(Mu.Cone, Mu.Cone.Alpha);
        React(Mu.Rod, Mu.Cone.Alpha, Mu.Rod.Channel);
        React(Mu.Cone, Mu.Cone.Alpha, Mu.Cone.Channel);
        _mintToCap();
    }

    function Rho() public {
        Torque(Mu.Cone, Mu.Rod.Eta);
        Amplify(Mu.Cone, Mu.Upsilon);
        Sustain(Mu.Cone, Mu.Ohm);
        React(Mu.Cone, Mu.Pi, Mu.Cone.Dynamo);
        React(Mu.Rod, Mu.Pi, Mu.Rod.Dynamo);
        Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;
        _mintToCap();
    }

    function View() public view returns(Faung memory) {
        return Mu;
    }

    function Generate() public returns(uint64) {
        Amplify(Mu.Cone, Mu.Upsilon);
        Sustain(Mu.Cone, Mu.Ohm);
        React(Mu.Cone, Mu.Pi, Mu.Cone.Dynamo);
        React(Mu.Rod, Mu.Pi, Mu.Rod.Dynamo);
        Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;

        Mu.Upsilon = Mu.Upsilon ^ Mu.Ohm ^ Mu.Pi;
        
        _mintToCap();
        return Mu.Upsilon;
    }
}
