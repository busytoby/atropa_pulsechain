// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";
import "faung.sol";

contract NG is ERC20, ERC20Burnable, Ownable {
    Dynamic.Faung private Mu;

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

        RNG RNG1 = RNG(0xa96BcbeD7F01de6CEEd14fC86d90F21a36dE2143);
        //RNG RNG1 = RNG(0xBDA2A2cBD5E5E9B8BBd52614E94030D71F1dC54B);

        Conjecture.Fa memory Rod = Conjecture.New();
        Conjecture.Fa memory Cone = Conjecture.New();

        Mu = Dynamic.New(Rod, Cone, RNG1.Generate(), RNG1.Generate(), RNG1.Generate());

        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Upsilon(Mu.Upsilon, false);
        Pi();
        Rho();
        Upsilon(Mu.Upsilon, true);

        _mint(address(this), 1 * 10 ** decimals());
    }

    function Alpha(uint64 _a) public {
        Dynamic.Charge(Mu, _a);
        assert(Mu.Sigma > 4);
        Dynamic.Induce(Mu);
        Dynamic.Torque(Mu);
        Dynamic.Amplify(Mu);
        Dynamic.Sustain(Mu);
        Dynamic.React(Mu);
    }

    function Beta(uint64 _b) public {
        Conjecture.Torque(Mu.Rod, _b);
        Conjecture.Amplify(Mu.Rod, Mu.Rod.Alpha);
        Conjecture.Sustain(Mu.Rod, Mu.Rod.Alpha);
        Conjecture.React(Mu.Rod, Mu.Rod.Alpha, Mu.Cone.Dynamo);
        Conjecture.React(Mu.Cone, Mu.Rod.Alpha, Mu.Rod.Dynamo);
    }

    function Upsilon(uint64 _a, bool Phi) public {
        Mu.Upsilon = Phi ? _a ^ Mu.Ohm ^ Mu.Pi : _a ^ Mu.Ohm;
    }

    function Pi() public {
        Conjecture.Torque(Mu.Cone, Mu.Rod.Kappa);
        Conjecture.Amplify(Mu.Cone, Mu.Cone.Alpha);
        Conjecture.Sustain(Mu.Cone, Mu.Cone.Alpha);
        Conjecture.React(Mu.Rod, Mu.Cone.Alpha, Mu.Rod.Channel);
        Conjecture.React(Mu.Cone, Mu.Cone.Alpha, Mu.Cone.Channel);
    }

    function Rho() public {
        Conjecture.Torque(Mu.Cone, Mu.Rod.Eta);
        Mu.Upsilon = Mu.Cone.Alpha;
        Conjecture.Amplify(Mu.Cone, Mu.Upsilon);
        Mu.Ohm = Mu.Cone.Alpha;
        Conjecture.Sustain(Mu.Cone, Mu.Ohm);
        Mu.Pi = Mu.Cone.Alpha;
        Conjecture.React(Mu.Cone, Mu.Pi, Mu.Cone.Dynamo);
        Mu.Omicron = Mu.Cone.Kappa;
        Conjecture.React(Mu.Rod, Mu.Pi, Mu.Rod.Dynamo);
        Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;
    }

    function View() public view returns(Dynamic.Faung memory) {
        return Mu;
    }

    function AvailableForPurchase() public view returns(uint256) {
        return balanceOf(address(this));
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

    function Generate() public returns(uint64) {
        Conjecture.Amplify(Mu.Cone, Mu.Upsilon);
        Mu.Ohm = Mu.Cone.Alpha;
        Conjecture.Sustain(Mu.Cone, Mu.Ohm);
        Mu.Pi = Mu.Cone.Alpha;
        Conjecture.React(Mu.Cone, Mu.Pi, Mu.Cone.Dynamo);
        Mu.Omicron = Mu.Cone.Kappa;
        Conjecture.React(Mu.Rod, Mu.Pi, Mu.Rod.Dynamo);
        Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;

        Mu.Upsilon = Mu.Upsilon ^ Mu.Ohm ^ Mu.Pi;

        if(totalSupply() <= (1111111111 * 10 ** decimals()))
            _mint(address(this), 1 * 10 ** decimals());

        return Mu.Upsilon;
    }
}
