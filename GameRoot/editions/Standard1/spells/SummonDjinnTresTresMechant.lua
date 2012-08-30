cost = {0,1,4,0}
icon = "canard"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Invoquer Djinn tr�s tr�s m�chant"
	else
		return "Summon very evil djinn"
	end
end

function getDescription()
	if language == "french" then
		return "Invoque un djinn tr�s, tr�s m�chant!\n"..getUnitDescription("Djinn_tres_tres_mechant")
	else
		return "Summons a very, very evil djinn!\n"..getUnitDescription("Djinn_tres_tres_mechant")
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

-- this function receives map coords. as parameter, in a space-separated string
function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "Djinn_tres_tres_mechant")
end
