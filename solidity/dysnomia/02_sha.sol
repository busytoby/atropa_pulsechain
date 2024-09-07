// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./include/fa.sol";

contract SHA is DYSNOMIA {
    string public constant Type = "SHA";

    Fa private Mu;
    uint64 public Dynamo;

    constructor(string memory name, string memory symbol, address MathLib) DYSNOMIA(name, symbol, MathLib) {
        Seed();
        Tune();
        Augment();
    }

    function Augment() internal {
        mintToCap();
    }

    function View() public view returns(Fa memory) {
        return Mu;
    }

    function Seed() private {
        Mu.Base = Xiao.Random();
        Mu.Secret = Xiao.Random();
        Mu.Signal = Xiao.Random();
        _mintToCap();
    }

   function Tune() private {
        Mu.Channel = Xiao.modExp64(Mu.Base, Mu.Signal, MotzkinPrime);   
        _mintToCap();
    }

    function Fuse(uint64 _rho, uint64 Upsilon, uint64 Ohm) public onlyOwners {
        Mu.Base = Upsilon;
        Mu.Secret = Ohm;
        Mu.Signal = _rho;
        _mintToCap();
    }

    function Avail(uint64 Xi) public onlyOwners {
        Mu.Contour = Xiao.modExp64(Xi, Mu.Secret, MotzkinPrime);
        _mintToCap();
    }

    function Form(uint64 Chi) public onlyOwners {
        Mu.Base = Xiao.modExp64(Chi, Mu.Secret, MotzkinPrime);
        Tune();
        _mintToCap();
    }

    function Polarize() public onlyOwners {
        Mu.Pole = Xiao.modExp64(Mu.Base, Mu.Secret, MotzkinPrime);
        _mintToCap();
    }

    function Conjugate(uint64 Chi) public onlyOwners {
        Mu.Coordinate = Xiao.modExp64(Chi, Mu.Secret, MotzkinPrime);
        _mintToCap();
    }

    function Conify(uint64 _Beta) public onlyOwners {
        Mu.Identity = _Beta;
        Mu.Foundation = Xiao.modExp64(Mu.Base, Mu.Identity, MotzkinPrime);
        _mintToCap();
    }

    function Saturate(uint64 _Beta, uint64 Epsilon, uint64 Theta) public onlyOwners {
        if(Mu.Identity == 0) {
            Mu.Identity = _Beta;
            Mu.Foundation = Xiao.modExp64(Mu.Base, Mu.Identity, MotzkinPrime);
            _mintToCap();
        }
        
        uint64 Beta = Xiao.modExp64(Epsilon, Mu.Identity, MotzkinPrime);
        uint64 Rho = Xiao.modExp64(Theta, Mu.Identity, MotzkinPrime);
        uint64 Eta = Xiao.modExp64(Epsilon, Mu.Signal, MotzkinPrime);

        Mu.Charge = Rho + Eta;       
        Mu.Chin = Beta + Eta; 
        Mu.Element = Beta + Mu.Charge;

        Dynamo = Xiao.modExp64(Theta, Mu.Signal, MotzkinPrime);
        Mu.Monopole = Xiao.modExp64(Mu.Chin, Mu.Identity, MotzkinPrime);
        _mintToCap();
    }

    function Bond() public onlyOwners {
        Dynamo = Xiao.modExp64(Mu.Base, Mu.Signal, Mu.Element);
        Mu.Pole = 0;
        _mintToCap();
    }

    function Adduct(uint64 _Phi) public returns(uint64) {
        _mintToCap();
        return Xiao.modExp64(_Phi, Mu.Signal, Mu.Element);
    }

    error ReactionZeroError(uint64 Eta, uint64 Kappa);
    function React(uint64 Pi, uint64 Theta) public returns(uint64, uint64) {
        uint64 Eta = Xiao.modExp64(Pi, Mu.Channel, Theta);
        uint64 Kappa = Xiao.modExp64(Pi, Theta, Mu.Channel);
        if(Eta == 0 || Kappa == 0) revert ReactionZeroError(Eta, Kappa);
        _mintToCap();
        return(Eta, Kappa);
    }
}