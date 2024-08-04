// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "01_sh.sol";

struct Fa {
        uint64 Base;
        uint64 Secret;
        uint64 Signal;
        uint64 Channel;
        uint64 Contour;
        uint64 Pole;
        uint64 Identity;
        uint64 Foundation;
        uint64 Element;
        uint64 Coordinate;
        uint64 Charge;
        uint64 Chin;
        uint64 Monopole;

        uint64 Nu;
}


contract SHA is SH {
    string public constant Type = "SHA";

    Fa private Mu;
    uint64 public Dynamo;

    constructor(string memory name, string memory symbol, address MathLib) ERC20(name, symbol) SH(MathLib, 131) MultiOwnable(msg.sender) {
        mintToCap();
        Seed();
        Tune();
    }

    function View() public view returns(Fa memory) {
        return Mu;
    }

    function Seed() private {
        Mu.Base = Xiao.Random();
        Mu.Secret = Xiao.Random();
        Mu.Signal = Xiao.Random();
    }

   function Tune() private {
        Mu.Channel = Xiao.modExp64(Mu.Base, Mu.Signal, MotzkinPrime);   
    }

    function Fuse(uint64 _rho, uint64 Upsilon, uint64 Ohm) public onlyOwners {
        Mu.Base = Upsilon;
        Mu.Secret = Ohm;
        Mu.Signal = _rho;
    }

    function Avail(uint64 Xi) public onlyOwners {
        Mu.Contour = Xiao.modExp64(Xi, Mu.Secret, MotzkinPrime);
    }

    function Form(uint64 Chi) public onlyOwners {
        Mu.Base = Xiao.modExp64(Chi, Mu.Secret, MotzkinPrime);
        Tune();
    }

    function Polarize() public onlyOwners {
        Mu.Pole = Xiao.modExp64(Mu.Base, Mu.Secret, MotzkinPrime);
    }

    function Conjugate(uint64 Chi) public onlyOwners {
        Mu.Coordinate = Xiao.modExp64(Chi, Mu.Secret, MotzkinPrime);
        // Chi = 0;
    }

    function Conify(uint64 _Beta) public onlyOwners {
        assert(Mu.Nu == 0);
        Mu.Identity = _Beta;
        Mu.Foundation = Xiao.modExp64(Mu.Base, Mu.Identity, MotzkinPrime);
        Mu.Nu = 1;
    }

    function Saturate(uint64 _Beta, uint64 Epsilon, uint64 Theta) public onlyOwners {
        if(Mu.Nu == 0) {
            Mu.Identity = _Beta;
            Mu.Foundation = Xiao.modExp64(Mu.Base, Mu.Identity, MotzkinPrime);
        }
        assert(Mu.Nu <= 1);
        
        uint64 Beta = Xiao.modExp64(Epsilon, Mu.Identity, MotzkinPrime);
        uint64 Rho = Xiao.modExp64(Theta, Mu.Identity, MotzkinPrime);
        uint64 Eta = Xiao.modExp64(Epsilon, Mu.Signal, MotzkinPrime);

        Mu.Charge = Rho + Eta;       
        Mu.Chin = Beta + Eta; 
        Mu.Element = Beta + Mu.Charge;

        Dynamo = Xiao.modExp64(Theta, Mu.Signal, MotzkinPrime);
        Mu.Monopole = Xiao.modExp64(Mu.Chin, Mu.Identity, MotzkinPrime);
    }

    function Bond() public onlyOwners {
        Dynamo = Xiao.modExp64(Mu.Base, Mu.Signal, Mu.Element);
        Mu.Pole = 0;
    }

    function Adduct(uint64 _Phi) public returns(uint64) {
        return Xiao.modExp64(_Phi, Mu.Signal, Mu.Element);
    }

    function Torque(uint64 Pi) public returns(uint64) {
        uint64 Alpha = Xiao.modExp64(Pi, Mu.Element, Mu.Channel);
        return Alpha;
    }

    function Flux(uint64 Pi) public returns(uint64) {
        uint64 Alpha = Xiao.modExp64(Pi, Mu.Monopole, Mu.Channel);
        return Alpha;
    }

    function React(uint64 Pi, uint64 Theta) public returns(uint64, uint64) {
        uint64 Eta = Xiao.modExp64(Pi, Mu.Channel, Theta);
        uint64 Kappa = Xiao.modExp64(Pi, Theta, Mu.Channel);
        assert(Eta != 0 && Kappa != 0);
        mintToCap();
        return(Eta, Kappa);
    }
}