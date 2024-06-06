// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";
import "faung.sol";

contract RNG is ERC20, ERC20Burnable, Ownable {
    Dynamic.Faung private Mu;

    ERC20 private DaiToken;
    ERC20 private USDCToken;
    ERC20 private USDTToken;
    ERC20 private G5Token;
    ERC20 private PIToken;

    constructor() ERC20(/*name short=*/ unicode"Random Number Generator", /*symbol long=*/ unicode"RNG") Ownable(msg.sender) {
        DaiToken = ERC20(dai);
        USDCToken = ERC20(usdc);
        USDTToken = ERC20(usdt);
        G5Token = ERC20(G5Contract);
        PIToken = ERC20(PIContract);

        Conjecture.Fa memory Rod = Conjecture.New(605841066431434, 824993723223339, 543871960643842);
        Conjecture.Fa memory Cone = Conjecture.New(605841066431434, 706190044965693, 187758195120264);

        Mu = Dynamic.New(Rod, Cone, 314267673176633, 300042286926212, 658285068338874);

        Dynamic.Charge(Mu, Mu.Rod.Signal);
        assert(Mu.Sigma > 4);
        Dynamic.Induce(Mu);
        Dynamic.Torque(Mu);
        Dynamic.Amplify(Mu);
        Dynamic.Sustain(Mu);
        Dynamic.React(Mu);

        Conjecture.Torque(Mu.Rod, Mu.Upsilon);
        Conjecture.Amplify(Mu.Rod, Mu.Rod.Alpha);
        Conjecture.Sustain(Mu.Rod, Mu.Rod.Alpha);
        Conjecture.React(Mu.Rod, Mu.Rod.Alpha, Mu.Cone.Dynamo);
        Conjecture.React(Mu.Cone, Mu.Rod.Alpha, Mu.Rod.Dynamo);

        Mu.Upsilon = Mu.Upsilon ^ Mu.Ohm;

        Conjecture.Torque(Mu.Cone, Mu.Rod.Kappa);
        Conjecture.Amplify(Mu.Cone, Mu.Cone.Alpha);
        Conjecture.Sustain(Mu.Cone, Mu.Cone.Alpha);
        Conjecture.React(Mu.Rod, Mu.Cone.Alpha, Mu.Rod.Channel);
        Conjecture.React(Mu.Cone, Mu.Cone.Alpha, Mu.Cone.Channel);

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

        Mu.Upsilon = Mu.Upsilon ^ Mu.Ohm ^ Mu.Pi;

        _mint(address(this), 1 * 10 ** decimals());
    }

    function View() public view returns(Dynamic.Faung memory) {
        return Mu;
    }

    function AvailableForPurchase() public view returns(uint256) {
        return balanceOf(address(this));
    }

    function BuyWithDAI(uint256 amount) public {
        assert(AvailableForPurchase() >= amount);
        bool success1 = DaiToken.transferFrom(msg.sender, address(this), amount * 10 ** DaiToken.decimals());
        require(success1, unicode"Need Approved DAI");
        transfer(msg.sender, amount * 10 ** decimals());
    }

    function BuyWithUSDC(uint256 amount) public {
        assert(AvailableForPurchase() >= amount);
        bool success1 = USDCToken.transferFrom(msg.sender, address(this), amount * 10 ** USDCToken.decimals());
        require(success1, unicode"Need Approved USDC");
        transfer(msg.sender, amount * 10 ** decimals());
    }

    function BuyWithUSDT(uint256 amount) public {
        assert(AvailableForPurchase() >= amount);
        bool success1 = USDTToken.transferFrom(msg.sender, address(this), amount * 10 ** USDCToken.decimals());
        require(success1, unicode"Need Approved USDT");
        transfer(msg.sender, amount * 10 ** decimals());
    }

    function BuyWithG5(uint256 amount) public {
        assert(AvailableForPurchase() >= amount);
        bool success1 = G5Token.transferFrom(msg.sender, address(this), (amount * 10 ** G5Token.decimals()) / 4);
        require(success1, unicode"Need Approved Gimme5");
        transfer(msg.sender, amount * 10 ** decimals());
    }

    function BuyWithPI(uint256 amount) public {
        assert(AvailableForPurchase() >= amount);
        bool success1 = PIToken.transferFrom(msg.sender, address(this), (amount * 10 ** PIToken.decimals() / 212));
        require(success1, unicode"Need Approved pINDEPENDENCE");
        transfer(msg.sender, amount * 10 ** decimals());
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
